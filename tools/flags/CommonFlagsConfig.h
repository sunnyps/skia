/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SK_COMMON_FLAGS_CONFIG_H
#define SK_COMMON_FLAGS_CONFIG_H

#include "tools/flags/CommandLineFlags.h"
#include "tools/gpu/GrContextFactory.h"

DECLARE_string(config);

class SkCommandLineConfigGpu;
class SkCommandLineConfigSvg;

// SkCommandLineConfig represents a Skia rendering configuration string.
// The string has following form:
// tag:
//   [via-]*backend
// where 'backend' consists of chars excluding hyphen
// and each 'via' consists of chars excluding hyphen.
class SkCommandLineConfig {
public:
    SkCommandLineConfig(const SkString&           tag,
                        const SkString&           backend,
                        const SkTArray<SkString>& viaParts);
    virtual ~SkCommandLineConfig();
    virtual const SkCommandLineConfigGpu* asConfigGpu() const { return nullptr; }
    virtual const SkCommandLineConfigSvg* asConfigSvg() const { return nullptr; }
    const SkString&                       getTag() const { return fTag; }
    const SkString&                       getBackend() const { return fBackend; }
    sk_sp<SkColorSpace>                   refColorSpace() const { return fColorSpace; }
    const SkTArray<SkString>&             getViaParts() const { return fViaParts; }

private:
    SkString            fTag;
    SkString            fBackend;
    sk_sp<SkColorSpace> fColorSpace;
    SkTArray<SkString>  fViaParts;
};

// SkCommandLineConfigGpu is a SkCommandLineConfig that extracts information out of the backend
// part of the tag. It is constructed tags that have:
// * backends of form "gpu[option=value,option2=value,...]"
// * backends that represent a shorthand of above (such as "glmsaa16" representing
// "gpu(api=gl,samples=16)")
class SkCommandLineConfigGpu : public SkCommandLineConfig {
public:
    enum class SurfType { kDefault, kBackendTexture, kBackendRenderTarget };
    typedef sk_gpu_test::GrContextFactory::ContextType      ContextType;
    typedef sk_gpu_test::GrContextFactory::ContextOverrides ContextOverrides;

    SkCommandLineConfigGpu(const SkString&           tag,
                           const SkTArray<SkString>& viaParts,
                           ContextType               contextType,
                           bool                      fakeGLESVer2,
                           uint32_t                  surfaceFlags,
                           int                       samples,
                           SkColorType               colorType,
                           SkAlphaType               alphaType,
                           bool                      useStencilBuffers,
                           bool                      testThreading,
                           int                       testPersistentCache,
                           bool                      testPrecompile,
                           bool                      useDDLSink,
                           bool                      OOPRish,
                           bool                      reducedShaders,
                           bool                      useGraphite,
                           SurfType);

    const SkCommandLineConfigGpu* asConfigGpu() const override { return this; }
    ContextType                   getContextType() const { return fContextType; }
    ContextOverrides              getContextOverrides() const { return fContextOverrides; }
    uint32_t      getSurfaceFlags() const { return fSurfaceFlags; }
    int           getSamples() const { return fSamples; }
    SkColorType   getColorType() const { return fColorType; }
    SkAlphaType   getAlphaType() const { return fAlphaType; }
    bool          getTestThreading() const { return fTestThreading; }
    int           getTestPersistentCache() const { return fTestPersistentCache; }
    bool          getTestPrecompile() const { return fTestPrecompile; }
    bool          getUseDDLSink() const { return fUseDDLSink; }
    bool          getOOPRish() const { return fOOPRish; }
    bool          getReducedShaders() const { return fReducedShaders; }
    bool          getUseGraphite() const { return fUseGraphite; }
    SurfType      getSurfType() const { return fSurfType; }

private:
    ContextType         fContextType;
    ContextOverrides    fContextOverrides;
    uint32_t            fSurfaceFlags;
    int                 fSamples;
    SkColorType         fColorType;
    SkAlphaType         fAlphaType;
    bool                fTestThreading;
    int                 fTestPersistentCache;
    bool                fTestPrecompile;
    bool                fUseDDLSink;
    bool                fOOPRish;
    bool                fReducedShaders;
    bool                fUseGraphite;
    SurfType            fSurfType;
};

// SkCommandLineConfigSvg is a SkCommandLineConfig that extracts information out of the backend
// part of the tag. It is constructed tags that have:
// * backends of form "svg[option=value,option2=value,...]"
class SkCommandLineConfigSvg : public SkCommandLineConfig {
public:
    SkCommandLineConfigSvg(const SkString& tag, const SkTArray<SkString>& viaParts, int pageIndex);
    const SkCommandLineConfigSvg* asConfigSvg() const override { return this; }

    int getPageIndex() const { return fPageIndex; }

private:
    int fPageIndex;
};

typedef SkTArray<std::unique_ptr<SkCommandLineConfig>, true> SkCommandLineConfigArray;
void ParseConfigs(const CommandLineFlags::StringArray& configList,
                  SkCommandLineConfigArray*            outResult);

#endif
