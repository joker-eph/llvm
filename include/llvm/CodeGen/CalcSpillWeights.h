//===---------------- lib/CodeGen/CalcSpillWeights.h ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_CODEGEN_CALCSPILLWEIGHTS_H
#define LLVM_CODEGEN_CALCSPILLWEIGHTS_H

#include "llvm/ADT/DenseMap.h"
#include "llvm/CodeGen/SlotIndexes.h"

namespace llvm {

  class LiveInterval;
  class LiveIntervals;
  class MachineBlockFrequencyInfo;
  class MachineLoopInfo;

  /// \brief Normalize the spill weight of a live interval
  ///
  /// The spill weight of a live interval is computed as:
  ///
  ///   (sum(use freq) + sum(def freq)) / (K + size)
  ///
  /// @param UseDefFreq Expected number of executed use and def instructions
  ///                   per function call. Derived from block frequencies.
  /// @param Size       Size of live interval as returnexd by getSize()
  ///
  static inline float normalizeSpillWeight(float UseDefFreq, unsigned Size) {
    // The constant 25 instructions is added to avoid depending too much on
    // accidental SlotIndex gaps for small intervals. The effect is that small
    // intervals have a spill weight that is mostly proportional to the number
    // of uses, while large intervals get a spill weight that is closer to a use
    // density.
    return UseDefFreq / (Size + 25*SlotIndex::InstrDist);
  }

  /// \brief Calculate auxiliary information for a virtual register such as its
  /// spill weight and allocation hint.
  class VirtRegAuxInfo {
    MachineFunction &MF;
    LiveIntervals &LIS;
    const MachineLoopInfo &Loops;
    const MachineBlockFrequencyInfo &MBFI;
    DenseMap<unsigned, float> Hint;
  public:
    VirtRegAuxInfo(MachineFunction &mf, LiveIntervals &lis,
                   const MachineLoopInfo &loops,
                   const MachineBlockFrequencyInfo &mbfi)
        : MF(mf), LIS(lis), Loops(loops), MBFI(mbfi) {}

    /// \brief (re)compute li's spill weight and allocation hint.
    void CalculateWeightAndHint(LiveInterval &li);
  };

  /// \brief Compute spill weights and allocation hints for all virtual register
  /// live intervals.
  void calculateSpillWeights(LiveIntervals &LIS,
                             MachineFunction &MF,
                             const MachineLoopInfo &MLI,
                             const MachineBlockFrequencyInfo &MBFI);

}

#endif // LLVM_CODEGEN_CALCSPILLWEIGHTS_H
