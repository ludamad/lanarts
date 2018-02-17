#pragma once
#include "oxygine-include.h"
#include "Tween.h"
#include "PostProcess.h"

namespace oxygine
{
    class TweenAlphaFade: public TweenProxy
    {
    public:
        TweenAlphaFade(bool fadeIn, const PostProcessOptions& opt = PostProcessOptions());
    };
}