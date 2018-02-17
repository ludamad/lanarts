#include "TweenGlow.h"
#include "STDMaterial.h"
#include "Actor.h"
#include "RenderState.h"

#include "core/gl/VertexDeclarationGL.h"



namespace oxygine
{



    class TweenGlowImpl : public TweenPostProcess
    {
    public:
        Color _color;
        int _downsample;

        TweenGlowImpl(const Color& c, const PostProcessOptions& opt) : TweenPostProcess(opt), _color(c), _downsample(1) {}

        void render(Actor* actor, const RenderState& rs) override
        {
            STDMaterial* mat = STDMaterial::instance;
            STDRenderer* renderer = mat->getRenderer();


            RenderState r = rs;
            r.material = mat;
            actor->setMaterial(_prevMaterial);
            actor->render(r);
            actor->setMaterial(this);


            RectF src(0, 0,
                      _pp._screen.getWidth() / (float)_pp._rt->getWidth() / _downsample,
                      _pp._screen.getHeight() / (float)_pp._rt->getHeight() / _downsample);

            RectF dest = _pp._screen.cast<RectF>();

            renderer->setBlendMode(blend_premultiplied_alpha);


            AffineTransform tr = _pp._transform * _actor->computeGlobalTransform();
            renderer->setTransform(tr);
            renderer->beginElementRendering(true);
            Color color = Color(Color::White).withAlpha(255).premultiplied();
            color = Color::White;
            renderer->setBlendMode(blend_add);
            renderer->drawElement(_pp._rt, color.rgba(), src, dest);
            renderer->drawBatch();
        }

        void _renderPP() override
        {
            PostProcess::initShaders();

            int w = _pp._screen.size.x;
            int h = _pp._screen.size.y;


            IVideoDriver* driver = IVideoDriver::instance;
            const VertexDeclarationGL* decl = static_cast<const VertexDeclarationGL*>(IVideoDriver::instance->getVertexDeclaration(vertexPCT2::FORMAT));

            _downsample = 1;


            spNativeTexture rt = _pp._rt;
            spNativeTexture rt2 = getRTManager().get(0, w, h, _pp._format);

#if 0
            driver->setShaderProgram(PostProcess::shaderBlit);
            pass(rt, Rect(0, 0, w, h), rt2, Rect(0, 0, w / 2, h / 2));

            w /= 2;
            h /= 2;
            _downsample *= 2;
#endif

#if 0

            rt = getRTManager().get(w / 2, h / 2, _pp._format);
            _pp._rt = rt;
            pass(rt2, Rect(0, 0, w, h), rt, Rect(0, 0, w / 2, h / 2));

            w /= 2;
            h /= 2;
            _downsample *= 2;
#endif


            Rect rc(0, 0, w, h);


            driver->setShaderProgram(PostProcess::shaderBlurH);
            driver->setUniform("step", 1.0f / rt->getWidth());
            pass(rt, rc, rt2, rc);


            int alpha = lerp(0, 255, _progress);
            //log::messageln("tween alpha %d", alpha);

            Color c = _color.withAlpha(64).premultiplied();
            driver->setShaderProgram(PostProcess::shaderBlurV);
            driver->setUniform("step", 1.0f / rt2->getHeight());

            pass(rt2, rc, rt, rc, c);
        }
    };


    TweenGlow::TweenGlow(const Color& color, const PostProcessOptions& opt) : TweenProxy(new TweenGlowImpl(color, opt))
    {
    }
}