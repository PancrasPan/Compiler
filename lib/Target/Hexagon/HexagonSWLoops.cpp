//
// An implementation of the Swing Modulo Scheduling (SMS) software pipeliner.
//
/*
The algorithm followed by SMS consists of the following three steps that are described in
detail below:
1. Computation and analysis of the dependence graph.
2.Ordering of the nodes.
3.Scheduling.


*/

/*
#include "llvm/ADT/SetVector.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/ValueTracking.h"
#include "llvm/Analysis/CodeMetrics.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/CodeGen/LiveIntervalAnalysis.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineLoopInfo.h"
#include "llvm/CodeGen/MachineDominators.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/RegisterClassInfo.h"
#include "llvm/CodeGen/RegisterPressure.h"
#include "llvm/CodeGen/ScheduleDAGInstrs.h"
#include "llvm/IR/Constant.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/Pass.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Target/TargetInstrInfo.h"
#include <vector>
#include <iostream>
#include <climits>
#include <algorithm>
using namespace llvm;

#define DEBUG_TYPE "swpipeline"
// Helper function (copied from LoopVectorize.cpp)
static void addInnerLoop(Loop &L, SmallVectorImpl<Loop *> &V) {
	if (L.empty())
		return V.push_back(&L);

	for (Loop *InnerL : L)
		addInnerLoop(*InnerL, V);
}
namespace llvm {
	void initializeDSPSWLoopsPass(PassRegistry&);
}
cl::opt<bool> hexsw("hex-sw", cl::init(true), cl::Hidden, cl::desc("hexagon sw"));

namespace {
	class NodeSet;

	typedef SmallVector<NodeSet, 8> NodeSetType;

	class DSPSWLoops : public MachineFunctionPass{
	public:
		static char ID;
		explicit DSPSWLoops() :MachineFunctionPass(ID), MF(nullptr), MLI(nullptr), MDT(nullptr), AA(nullptr),
			TII(nullptr){
			initializeDSPSWLoopsPass(*PassRegistry::getPassRegistry());
		}
		bool runOnMachineFunction(MachineFunction &mf) override{
			this->MF = &mf;
			//CMA = &getAnalysis<CostModelAnalysis>();
			MLI = &getAnalysis<MachineLoopInfo>();
			MDT = &getAnalysis<MachineDominatorTree>();
			AA = &getAnalysis<AliasAnalysis>();
			RegClassInfo.runOnMachineFunction(mf);
			TII = MF->getTarget().getInstrInfo();
			bool Changed;
			for (MachineLoopInfo::iterator I = MLI->begin(), E = MLI->end(); I != E; I++)
			{
				MachineLoop *L = *I;

				DEBUG(dbgs() << "*****************************Start Perform SoftWare Pipeling****************************" << "\n");
				Changed = Process(L);
			}
			return true;
		}

		const char* getPassName() const override{
			return "DSP SoftWare Pipeline Pass";
		}
		void getAnalysisUsage(AnalysisUsage &AU) const override{
			AU.setPreservesCFG();
			AU.addRequiredID(MachineDominatorsID);
			AU.addRequired<MachineLoopInfo>();
			AU.addRequired<AliasAnalysis>();
			AU.addRequired<LiveIntervals>();
			AU.addPreserved<LiveIntervals>();
			MachineFunctionPass::getAnalysisUsage(AU);
		}
	public:
		MachineFunction *MF;
		const MachineLoopInfo *MLI;
		const MachineDominatorTree *MDT;
		DependenceAnalysis *DA;
		AliasAnalysis *AA;
		RegisterClassInfo RegClassInfo;
		const TargetInstrInfo *TII;
		bool Process(MachineLoop *L);
		bool swingModuloScheduler(MachineLoop *L);
		bool canPipelineLoop(MachineLoop *L);

		class InstructionTrace
		{
		public:
			InstructionTrace() :weight(0){}
			InstructionTrace(const InstructionTrace  &IT) :trace(IT.trace), weight(IT.weight){}
			//~InstructionTrace();

			void add(Instruction *I){
				trace.push_back(I);
				weight += 1;
			}

			unsigned getWeight(){
				return weight;
			}

			const SmallVector<Instruction*, 8> &data() const {
				return trace;
			}

			size_t size() {
				return trace.size();
			}

			const Instruction* find(Instruction *I) const {
				for (auto II = trace.begin(), E = trace.end(); II != E; II++)
				if (*II == I) return *II;
				return nullptr;

			}

			//overload the < operator  I < J == I.<(J) return a bool value

			bool operator < (const InstructionTrace &I)const {
				return weight <= I.weight&&this != &I;
			}
		private:
			SmallVector<Instruction*, 8> trace;
			unsigned weight;

		};

		typedef std::set<InstructionTrace> CycleSet;
		//caculate the resource minimal initial interval
		unsigned calculateResMII(MachineLoop *L);

		//caculate the recursion minimal initial interval
		unsigned calculateRecMII(MachineLoop *L, CycleSet &C);


		// Helper function to find loop dependency cycles through phi nodes
		void getPhiCycles(MachineInstr *I, const PHINode *Phi, InstructionTrace trace, CycleSet &cycles);

		bool getConnectingNodes(Instruction *I,
			const BasicBlock *MBB,
			DenseMap<Instruction*, bool> &VisitedNodes,
			std::vector<Instruction *> &connectionNodes,
			bool direction
			);
	};


	//this class is used to build dependence graph
	class SwingSchedulerDAG : public ScheduleDAGInstrs{
		DSPSWLoops &pass;
		unsigned MII;
		bool isScheduled;
		MachineLoop &L;
		LiveIntervals &LIS;


		/// A toplogical ordering of the SUnits, which is needed for changing
		/// dependences and iterating over the SUnits.
		ScheduleDAGTopologicalSort Topo;

		struct NodeInfo{
			int ASAP;
			int ALAP;
			NodeInfo() :ASAP(0), ALAP(0){};
		};

		SetVector<SUnit*> NodeOrder;
		std::vector<NodeInfo> ScheduleInfo;
	public:

		/// Helper class to implement Johnson's circuit finding algorithm.
		class Circuits {
			std::vector<SUnit> &SUnits;
			SetVector<SUnit *> Stack;
			BitVector Blocked;
			SmallVector<SmallPtrSet<SUnit *, 4>, 10> B;
			SmallVector<SmallVector<int, 4>, 16> AdjK;
			unsigned NumPaths;
			static unsigned MaxPaths;

		public:
			Circuits(std::vector<SUnit> &SUs)
				: SUnits(SUs), Stack(), Blocked(SUs.size()), B(SUs.size()),
				AdjK(SUs.size()) {}
			/// Reset the data structures used in the circuit algorithm.
			void reset() {
				Stack.clear();
				Blocked.reset();
				B.assign(SUnits.size(), SmallPtrSet<SUnit *, 4>());
				NumPaths = 0;
			}
			void createAdjacencyStructure(SwingSchedulerDAG *DAG);
			bool circuit(int V, int S, NodeSetType &NodeSets, bool HasBackedge = false);
			void unblock(int U);
		};


	private:
		const RegisterClassInfo &RegClassInfo;
	public:
		SwingSchedulerDAG(DSPSWLoops &P, MachineLoop *L, const RegisterClassInfo &rci, LiveIntervals &LIV)
			:ScheduleDAGInstrs(*P.MF, *P.MLI, *P.MDT, false, false, &LIV), pass(P), MII(0), isScheduled(false),
			L(*L), Topo(SUnits, &ExitSU), RegClassInfo(rci), LIS(LIV){
			TII = P.MF->getTarget().getInstrInfo();
		}

		void schedule();
		int getASAP(SUnit *Node) { return ScheduleInfo[Node->NodeNum].ASAP; }

		/// Return the latest time an instruction my be scheduled.
		int getALAP(SUnit *Node) { return ScheduleInfo[Node->NodeNum].ALAP; }

		/// The mobility function, which the the number of slots in which
		/// an instruction may be scheduled.
		int getMOV(SUnit *Node) { return getALAP(Node) - getASAP(Node); }

		/// The depth, in the dependence graph, for a node.
		int getDepth(SUnit *Node) { return Node->getDepth(); }

		/// The height, in the dependence graph, for a node.
		int getHeight(SUnit *Node) { return Node->getHeight(); }


	private:
		void findRecCircuits(NodeSetType nodeset);

		bool isLoopCarriedOrder(SUnit *Source, const SDep &Dep, bool isSucc);

		bool computeDelta(MachineInstr *MI, unsigned &Delta);

	};

	class NodeSet{
		SetVector<SUnit*> Nodes;
		bool hasRecurrence;
		unsigned RecMII;
		int MaxMov;
		int MaxDepth;
		unsigned Colocate;
		SUnit *ExceedPressure;
	public:
		typedef SetVector<SUnit*>::iterator iterator;
		typedef SetVector<SUnit*>::const_iterator const_iterator;
		NodeSet() :Nodes(), hasRecurrence(false), RecMII(0), MaxMov(0), MaxDepth(0), Colocate(0), ExceedPressure(nullptr){}

		template <typename It>
		NodeSet(It S, It E)
			: Nodes(S, E), HasRecurrence(true), RecMII(0), MaxMOV(0), MaxDepth(0),
			Colocate(0), ExceedPressure(nullptr) {}
		unsigned size() const { return Nodes.size(); }

		void clear() {
			Nodes.clear();
			RecMII = 0;
			hasRecurrence = false;
			MaxMov = 0;
			MaxDepth = 0;
			Colocate = 0;
			ExceedPressure = nullptr;
		}
		iterator begin(){
			return Nodes.begin();
		}
		const_iterator begin() const {
			return Nodes.begin();
		}

		iterator end(){
			return Nodes.end();
		}
		const iterator end() const {
			return Nodes.end();
		}
		void print(raw_ostream &os) const {
			os << "Num nodes " << size() << " rec " << RecMII << " mov " << MaxMov
				<< " depth " << MaxDepth << " col " << Colocate << "\n";
			for (iterator I = begin(), E = end(); I != E; ++I)
				os << "   SU(" << (*I)->NodeNum << ") " << *((*I)->getInstr());
			os << "\n";
		}

		void dump() const { print(dbgs()); }
	};
} // end anonymous namespace

char DSPSWLoops::ID = 0;

INITIALIZE_PASS_BEGIN(DSPSWLoops, "DSPSWLoops", "SoftWare Pipeline", false, false)
INITIALIZE_PASS_DEPENDENCY(MachineLoopInfo)
INITIALIZE_PASS_END(DSPSWLoops, "DSPSWLoops", "SoftWare Pipeline", false, false)


//******************************static help function*********************************************

/// Return true if the dependence is an order dependence between non-Phis.
static bool isOrder(SUnit *Source, const SDep &Dep) {
	if (Dep.getKind() != SDep::Order)
		return false;
	return (!Source->getInstr()->isPHI() &&
		!Dep.getSUnit()->getInstr()->isPHI());
}

/// Return the Phi register value that comes the the loop block.
static unsigned getLoopPhiReg(MachineInstr *Phi, MachineBasicBlock *LoopBB) {
	for (unsigned i = 1, e = Phi->getNumOperands(); i != e; i += 2)
	if (Phi->getOperand(i + 1).getMBB() == LoopBB)
		return Phi->getOperand(i).getReg();
	return 0;
}


static bool isSlot01_Mov(MachineInstr *MI){
	switch (MI->getOpcode())
	{
	default:return false;
		break;
	}
}


// Helper function to find the nodes located at any path between the previous
// and current recurrence.
bool DSPSWLoops::getConnectingNodes(Instruction *I,
	const BasicBlock *B,
	DenseMap<Instruction *, bool> &VisitedNodes,
	std::vector<Instruction *> &connectingNodes,
	bool direction)
{
	// Do not recurse over nodes outside of the current loop body
	if (I->getParent() != B) return false;

	// Recurse until a previously visited node is found
	if (VisitedNodes[I]) return true;

	// Recurse through operands/uses depending on direction
	bool found = false;
	if (direction) {
		// avoid backedges
		if (isa<PHINode>(I)) return false;

		// search upwards
		for (auto &O : I->operands()) {
			Instruction *II = dyn_cast<Instruction>(O);
			if (II)
				found |= getConnectingNodes(II, B, VisitedNodes, connectingNodes, direction);
		}
	}
	else {
		// search downwards
		for (auto U : I->users()) {
			if (isa<PHINode>(U)) continue;
			Instruction *II = dyn_cast<Instruction>(U);
			if (II)
				found |= getConnectingNodes(II, B, VisitedNodes, connectingNodes, direction);
		}
	}

	// Add current node to the visited list and to the connecting nodes if a path was found
	if (found) {
		VisitedNodes[I] = true;
		connectingNodes.push_back(I);
	}

	return found;
}


//********************************
bool DSPSWLoops::canPipelineLoop(MachineLoop *L){
	// Check if loop body has no control flow (single BasicBlock)
	unsigned NumBlocks = L->getNumBlocks();
	if (NumBlocks != 1){
		return false;
	}
	return true;
}



//4 slots
//see DSPSchedule.td





unsigned DSPSWLoops::calculateRecMII(MachineLoop *L, CycleSet &C){
	MachineBasicBlock *LoopBody = L->getBlocks()[0];
	for (auto I = LoopBody->begin(), E = LoopBody->end(); I != E; I++)
	{
		if (!I->isPHI())
		{
			continue;
		}
		InstructionTrace tr;
		//getPhiCycles(Phi, Phi, tr, C);
	}
	return 2;
}

bool DSPSWLoops::swingModuloScheduler(MachineLoop *L){
	if (L->getBlocks().size() != 1) llvm_unreachable("SMS works on single block only");
	SwingSchedulerDAG SMS(*this, L, RegClassInfo, getAnalysis<LiveIntervals>());
	MachineBasicBlock *MBB = L->getBlocks()[0];
	DEBUG(dbgs() << "**********start build scheduler for " << MBB->getName() << "block" << "\n");
	SMS.startBlock(MBB);
	unsigned size = MBB->size();
	unsigned size2 = 0;
	for (MachineBasicBlock::iterator I = MBB->getFirstTerminator(),
		E = MBB->instr_end();
		I != E; ++I, --size);

	for (MachineBasicBlock::iterator i = MBB->getFirstNonPHI(), e = MBB->getFirstTerminator(); i != e; i++)
	{
		size2++;
	}
	std::cout << "size	" << size << std::endl;
	std::cout << "size2	" << size2 << std::endl;
	//SMS.enterRegion(MBB, MBB->getFirstNonPHI(), MBB->getFirstTerminator(), size2);
	SMS.enterRegion(MBB, MBB->begin(), MBB->getFirstTerminator(), size);
	SMS.schedule();
	SMS.exitRegion();
	SMS.finishBlock();
	return true;
}

void SwingSchedulerDAG::schedule(){
	AliasAnalysis *AA = pass.AA;
	buildSchedGraph(AA);
	Topo.InitDAGTopologicalSorting();
	for each (auto SU in SUnits)
	{
		std::cout << "su op" << SU.getInstr()->getOpcode() << std::endl;
	}
	NodeSetType NodeSets;
	Circuits Cir(SUnits);
	Cir.createAdjacencyStructure(this);

	for (int i = 0, e = SUnits.size(); i != e; ++i) {
		Cir.reset();
		Cir.circuit(i, i, NodeSets);
	}
}

//create the adjacency structure of the nodes in the graph
void SwingSchedulerDAG::Circuits::createAdjacencyStructure(SwingSchedulerDAG *DAG){
	BitVector Added(SUnits.size() + 1);
	for (int i = 0, e = SUnits.size(); i != e; i++){
		Added.reset();
		// Add any successor to the adjacency matrix and exclude duplicates.
		for each (auto &SI in SUnits[i].Succs)
		{
			if (SI.getKind() == SDep::Anti&&!SI.getSUnit()->getInstr()->isPHI())
				continue;

			// N represent position in SUnits
			int N = SI.getSUnit()->NodeNum;
			unsigned Op = SI.getSUnit()->getInstr()->getOpcode();
			if (N >= 0 && !Added.test(N))
			{
				AdjK[i].push_back(N);
				Added.set(N);
			}
		}

		// A chain edge between a store and a load is treated as a back-edge in the
		// adjacency matrix.
		for (auto &PI : SUnits[i].Preds) {
			if (!SUnits[i].getInstr()->mayStore() ||
				!DAG->isLoopCarriedOrder(&SUnits[i], PI, false))
				continue;
			if (PI.getKind() == SDep::Order && PI.getSUnit()->getInstr()->mayLoad()) {
				int N = PI.getSUnit()->NodeNum;
				if (N >= 0 && !Added.test(N)) {
					AdjK[i].push_back(N);
					Added.set(N);
				}
			}
		}
	}
}

/// Identify an elementary circuit in the dependence graph.
bool SwingSchedulerDAG::Circuits::circuit(int V, int S, NodeSetType &NodeSets, bool hasBackEage){
	return false;
}

bool SwingSchedulerDAG::isLoopCarriedOrder(SUnit *Source, const SDep &Dep,
	bool isSucc) {
	if (!isOrder(Source, Dep) || Dep.isArtificial())
		return false;

	//if (!SwpPruneLoopCarried)
	//return true;

	MachineInstr *SI = Source->getInstr();
	MachineInstr *DI = Dep.getSUnit()->getInstr();
	if (!isSucc)
		std::swap(SI, DI);
	assert(SI != nullptr && DI != nullptr && "Expecting SUnit with an MI.");

	// Assume ordered loads and stores may have a loop carried dependence.
	if (SI->hasUnmodeledSideEffects() || DI->hasUnmodeledSideEffects() ||
		SI->hasOrderedMemoryRef() || DI->hasOrderedMemoryRef())
		return true;

	// Only chain dependences between a load and store can be loop carried.
	if (!DI->mayStore() || !SI->mayLoad())
		return false;

	unsigned DeltaS, DeltaD;
	if (!computeDelta(SI, DeltaS) || !computeDelta(DI, DeltaD))
		return true;

	unsigned BaseRegS, OffsetS, BaseRegD, OffsetD;
	const TargetRegisterInfo *TRI = MF.getTarget().getRegisterInfo();
	if (!TII->getLdStBaseRegImmOfs(SI, BaseRegS, OffsetS, TRI) ||
		!TII->getLdStBaseRegImmOfs(DI, BaseRegD, OffsetD, TRI))
		return true;

	if (BaseRegS != BaseRegD)
		return true;

	uint64_t AccessSizeS = (*SI->memoperands_begin())->getSize();
	uint64_t AccessSizeD = (*DI->memoperands_begin())->getSize();

	// This is the main test, which checks the offset values and the loop
	// increment value to determine if the accesses may be loop carried.
	if (OffsetS >= OffsetD)
		return OffsetS + AccessSizeS > DeltaS;
	else if (OffsetS < OffsetD)
		return OffsetD + AccessSizeD > DeltaD;

	return true;
}

/// Return true if we can compute the amount the instruction changes
/// during each iteration. Set Delta to the amount of the change.
bool SwingSchedulerDAG::computeDelta(MachineInstr *MI, unsigned &Delta) {
	const TargetRegisterInfo *TRI = MF.getTarget().getRegisterInfo();
	unsigned BaseReg, Offset;
	if (!TII->getLdStBaseRegImmOfs(MI, BaseReg, Offset, TRI))
		return false;

	MachineRegisterInfo &MRI = MF.getRegInfo();
	// Check if there is a Phi. If so, get the definition in the loop.
	MachineInstr *BaseDef = MRI.getVRegDef(BaseReg);
	if (BaseDef && BaseDef->isPHI()) {
		BaseReg = getLoopPhiReg(BaseDef, MI->getParent());
		BaseDef = MRI.getVRegDef(BaseReg);
	}
	if (!BaseDef)
		return false;

	int D;
	if (!TII->getIncrementValue(BaseDef, D) || D < 0)
		return false;

	Delta = D;
	return true;
}
void  SwingSchedulerDAG::findRecCircuits(NodeSetType node){
	Circuits cir(SUnits);

}

bool DSPSWLoops::Process(MachineLoop *L){
	bool Changed = false;
	for (auto &InnerLoop : *L)
	{
		Changed |= Process(InnerLoop);
	}
	if (!canPipelineLoop(L))
		return Changed;
	Changed = swingModuloScheduler(L);
	return Changed;
}
//===----------------------------------------------------------------------===//
//                         Public Constructor Functions
//===----------------------------------------------------------------------===//

FunctionPass *llvm::createLoopPipelinePass() {
	return new DSPSWLoops();
}

*/