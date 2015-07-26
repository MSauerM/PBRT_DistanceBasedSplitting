
/*
    pbrt source code is Copyright(c) 1998-2015
                        Matt Pharr, Greg Humphreys, and Wenzel Jakob.

    This file is part of pbrt.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

    - Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    - Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
    IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
    TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#if defined(_MSC_VER)
#define NOMINMAX
#pragma once
#endif

#ifndef PBRT_INTEGRATORS_MLT_H
#define PBRT_INTEGRATORS_MLT_H
#include "stdafx.h"

// integrators/mlt.h*
#include "pbrt.h"
#include "integrator.h"
#include "sampler.h"
#include "spectrum.h"
#include "film.h"
#include "rng.h"

// MLTSampler Declarations
class MLTSampler : public Sampler {
  public:
    // MLTSampler Public Methods
    MLTSampler(int64_t mutationsPerPixel, int id, Float sigma,
               Float largeStepProb, int streamCount)
        : Sampler(mutationsPerPixel),
          rng(PCG32_DEFAULT_STATE, (uint64_t)id),
          sigma(sigma),
          largeStepProb(largeStepProb),
          streamCount(streamCount) {}
    Float Get1D();
    Point2f Get2D();
    std::unique_ptr<Sampler> Clone(int seed);
    void StartIteration();
    void Accept();
    void Reject();
    void StartStream(int index);
    int GetNextIndex() { return streamIndex + streamCount * sampleIndex++; }

  protected:
    // MLTSampler Private Declarations
    struct PrimarySample {
        Float value = 0;
        // PrimarySample Public Methods
        void Backup() {
            valueBackup = value;
            modifyBackup = lastModificationIteration;
        }
        void Restore() {
            value = valueBackup;
            lastModificationIteration = modifyBackup;
        }

        // PrimarySample Public Data
        int lastModificationIteration = 0;
        Float valueBackup = 0;
        int modifyBackup = 0;
    };

    // MLTSampler Private Methods
    void EnsureReady(int index);

    // MLTSampler Private Data
    RNG rng;
    Float sigma;
    Float largeStepProb;
    std::vector<PrimarySample> X;
    const int streamCount;
    int iteration = 0;
    bool largeStep = true;
    int lastLargeStepIteration = 0;
    int streamIndex;
    int sampleIndex;
};

// MLT Declarations
class MLTIntegrator : public Integrator {
  public:
    // MLTIntegrator Public Methods
    MLTIntegrator(std::shared_ptr<const Camera> camera, int maxDepth,
                  int nBootstrap, int nChains, int64_t mutationsPerPixel,
                  Float sigma, Float largeStepProb)
        : camera(camera),
          maxDepth(maxDepth),
          nBootstrap(nBootstrap),
          nChains(nChains),
          mutationsPerPixel(mutationsPerPixel),
          sigma(sigma),
          largeStepProb(largeStepProb){};
    void Render(const Scene &scene);
    Spectrum L(const Scene &scene, MemoryArena &arena, MLTSampler &sampler,
               int k, Point2f *samplePos);

  private:
    // MLTIntegrator Private Data
    std::shared_ptr<const Camera> camera;
    int maxDepth;
    int nBootstrap;
    int nChains;
    int64_t mutationsPerPixel;
    Float sigma;
    Float largeStepProb;
    std::unique_ptr<Distribution1D> lightDistr;
};

MLTIntegrator *CreateMLTIntegrator(const ParamSet &params,
                                   std::shared_ptr<const Camera> camera);

#endif  // PBRT_INTEGRATORS_MLT_H
