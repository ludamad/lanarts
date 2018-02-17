#pragma once
#include "oxygine-include.h"
#include "Sprite.h"

namespace oxygine
{
    DECLARE_SMART(ProgressBar, spProgressBar);

    class ProgressBar: public Sprite
    {
        INHERITED(Sprite);
    public:
        enum { PROGRESS_CHANGED = sysEventID('P', 'C', 'h') };

        enum direction
        {
            dir_0,//moveable right edge
            dir_90,//moveable top edge
            dir_180,//moveable left edge
            dir_270,//moveable bottom edge
            dir_radial_cw,
            __dir_radial_ccw//not implemented!!
        };

        DECLARE_COPYCLONE_NEW(ProgressBar);

        ProgressBar();
        ~ProgressBar();

        float       getProgress() const;
        direction   getDirection() const;

        /** setProgress
        @param value range [0.0f, 1.0f]
        */
        void setProgress(float value);
        void setDirection(direction dir);

        std::string dump(const dumpOptions&) const override;

        void serialize(serializedata* data) override;
        void deserialize(const deserializedata* data) override;

        typedef Property<float, float, ProgressBar, &ProgressBar::getProgress, &ProgressBar::setProgress> TweenProgress;

    private:
        void doRender(const RenderState&) override;

        virtual void _update();

        void animFrameChanged(const AnimationFrame& f) override;

        float _progress;
        direction _direction;
        AnimationFrame _originalFrame;
    };
}

EDITOR_INCLUDE(ProgressBar);