#include "ProgressBar.h"
#include "RenderState.h"
#include <sstream>
#include "Serialize.h"
#include "Material.h"
#include "STDRenderer.h"
#include "STDMaterial.h"

namespace oxygine
{
    void fill_tex_coord(vertexPCT2& vt, unsigned int rgba, const Vector2& pnt, float nu, float nv)
    {
        vt.color = rgba;
        vt.z = 0;
        vt.x = pnt.x;
        vt.y = pnt.y;
        vt.u = nu;
        vt.v = nv;
    }


    void rotateVector(Vector2& v, float rad)
    {
        Vector2 vh;

        vh.x = v.x * cosf(rad) - v.y * sinf(rad);
        vh.y = v.x * sinf(rad) + v.y * cosf(rad);

        v = vh;
    }

    float Angle(const Vector2& current, const Vector2* pRelative)
    {
        if (pRelative)
        {
            Vector2 s = current, t = *pRelative;

            s.normalize(); t.normalize();
            return acosf(s.dot(t));
        }
        else return atan2f(current.y, current.x);
    }

    ProgressBar::ProgressBar(): _progress(1.0f), _direction(dir_0)
    {
        AnimationFrame f(STDRenderer::white);
        f.setSize(200, 60);
        f.setDestRect(RectF(0, 0, 200, 60));
        setAnimFrame(f);
    }

    ProgressBar::~ProgressBar()
    {

    }

    void ProgressBar::copyFrom(const ProgressBar& src, cloneOptions opt)
    {
        inherited::copyFrom(src, opt);
        _progress = src._progress;
        _direction = src._direction;
        _originalFrame = src._originalFrame;
    }

    void ProgressBar::animFrameChanged(const AnimationFrame& f)
    {
        _originalFrame = f;
        _update();
        inherited::animFrameChanged(f);
    }

    void ProgressBar::_update()
    {
        if (!_frame.getDiffuse().base)
            return;

        RectF newSrc = _originalFrame.getSrcRect();
        RectF newDest = _originalFrame.getDestRect();

        float inv_progress = 1.0f - _progress;
        switch (_direction)
        {
            case dir_180:
                newSrc.pos.x += newSrc.size.x * inv_progress;
                newDest.pos.x += newDest.size.x * inv_progress;
            //break; do not break
            case dir_0:
                newSrc.size.x = newSrc.size.x * _progress;
                newDest.size.x = newDest.size.x * _progress;
                break;
            case dir_90:
                newSrc.pos.y += newSrc.size.y * inv_progress;
                newDest.pos.y += newDest.size.y * inv_progress;
            //break; do not break
            case dir_270:
                newSrc.size.y = newSrc.size.y * _progress;
                newDest.size.y = newDest.size.y * _progress;
                break;
            default:
                break;
        }

        Vector2 newSize = _originalFrame.getSize() * _progress;
        _frame.init(_frame.getResAnim(), _frame.getDiffuse(), newSrc, newDest, newSize);
        //_vstyle._material.srcRect = newSrc;
    }

    void ProgressBar::doRender(const RenderState& rs)
    {
        if (((_direction != __dir_radial_ccw) && (_direction != dir_radial_cw)) || (_progress == 1.0f))
        {
            inherited::doRender(rs);
            return;
        }


        Material::setCurrent(rs.material);
        //rs.material->doRender(this, rs);
        ///*

        STDRenderer* renderer = safeCast<STDMaterial*>(rs.material)->getRenderer();
        //STDRenderer* renderer = STDRenderer::instance;

        const Diffuse& df = _frame.getDiffuse();
        if (df.base)
        {
            renderer->setBlendMode(getBlendMode());
            renderer->setTexture(df.base, df.alpha, df.premultiplied);

            RectF destRect = inherited::getDestRect();

            RectF srcRect = _frame.getSrcRect();
            float u = srcRect.pos.x;
            float v = srcRect.pos.y;

            float du = srcRect.size.x;
            float dv = srcRect.size.y;

            u += du / 2.f;
            v += dv / 2.f;

            Vector2 pos = destRect.pos;
            const Vector2& size = destRect.size;
            pos += size / 2.f;

            Vector2 vecCenter(pos.x, pos.y);
            Vector2 vdiag = Vector2(pos.x + size.x / 2.f, pos.y - size.y / 2.f) - vecCenter;
            Vector2 vdiag2 = Vector2(pos.x + size.x / 2.f, pos.y + size.y / 2.f) - vecCenter;
            float lenDiag = vdiag.length();
            Vector2 vecCircle(pos.x, pos.y - lenDiag);

            Vector2 vecRad = vecCircle - vecCenter;

            float progress = _progress;

            float fP = MATH_PI * 2.f * progress;

            rotateVector(vecRad, fP);

            Vector2 p1(0.f, 0.f);
            Vector2 p2(0.f, 0.f);
            Vector2 p3(0.f, 0.f);

            Vector2 vert(0.f, -1.f);
            float fA1 = Angle(vdiag, &vert);
            float fA2 = Angle(vdiag2, &vdiag);

            const int MAX_TRI = 6;

            float u1, v1, u2, v2, u3, v3;
            float result = 0.f;

            float angles[ 6 ];
            angles[ 0 ] = fA1;
            angles[ 1 ] = fA2;
            angles[ 2 ] = fA1;
            angles[ 3 ] = fA1;
            angles[ 4 ] = fA2;
            angles[ 5 ] = fA1;

            for (int i = 0; i < MAX_TRI; i++)
            {
                float limitLo = 0.f;
                float limitHi = 0.f;
                for (int j = 0; j < i; j++)
                    limitLo += angles[ j ];

                limitHi = limitLo + angles[ i ];

                bool bOverHi = fP > limitHi;
                bool bOverLo = fP < limitLo;
                if (i && bOverLo)
                    continue;

                vertexPCT2 vertices[4];
                vertexPCT2* pv = vertices;

                switch (i)
                {
                    case 0:
                    {
                        result = bOverHi ? size.x / 2.f : vecRad.x;
                        p1 = Vector2(pos.x, pos.y);
                        p2 = Vector2(pos.x, pos.y - size.y / 2.f);
                        p3 = Vector2(pos.x + result, pos.y - size.y / 2.f);
                        float fPercent = result / size.x;
                        float fDU = du * fPercent;

                        u1 = u;
                        v1 = v;
                        u2 = u;
                        v2 = (v - dv / 2.f);
                        u3 = (u + fDU);
                        v3 = (v - dv / 2.f);
                    }
                    break;
                    case 1:
                    {
                        result = bOverHi ? size.y / 2.f : (vecRad.y) ;
                        p1 = Vector2(pos.x, pos.y);
                        p2 = Vector2(pos.x + size.x / 2.f, pos.y - size.y / 2.f);
                        p3 = Vector2(pos.x + size.x / 2.f, pos.y + result);
                        float fPercent = result / size.y;
                        float fDV = dv * fPercent;

                        u2 = u + du / 2.f;
                        v2 = (v - dv / 2.f);
                        u3 = u + du / 2.f;
                        v3 = (v + fDV);
                    }
                    break;
                    case 2:
                    {
                        result = bOverHi ? 0.f : vecRad.x ;
                        p1 = Vector2(pos.x, pos.y);
                        p2 = Vector2(pos.x + size.x / 2.f, pos.y + size.y / 2.f);
                        p3 = Vector2(pos.x + result, pos.y + size.y / 2.f);
                        float fPercent = result / size.x;
                        float fDU = du * fPercent;

                        u2 = u + du / 2.f;
                        v2 = (v + dv / 2.f);
                        u3 = u + fDU;
                        v3 = (v + dv / 2.f);
                    }
                    break;
                    case 3:
                    {
                        result = bOverHi ? (-size.x / 2.f) : vecRad.x ;
                        p1 = Vector2(pos.x, pos.y);
                        p2 = Vector2(pos.x , pos.y + size.y / 2.f);
                        p3 = Vector2(pos.x + result, pos.y + size.y / 2.f);
                        float fPercent = result / size.x;
                        float fDU = du * fPercent;

                        u2 = u;
                        v2 = (v + dv / 2.f);
                        u3 = u + fDU;
                        v3 = (v + dv / 2.f);
                    }
                    break;
                    case 4:
                    {
                        result = bOverHi ? (-size.y / 2.f) : vecRad.y ;
                        p1 = Vector2(pos.x, pos.y);
                        p2 = Vector2(pos.x - (size.x / 2.f) , pos.y + size.y / 2.f);
                        p3 = Vector2(pos.x - (size.x / 2.f), pos.y + result);
                        float fPercent = result / size.y;
                        float fDV = dv * fPercent;

                        u2 = u - du / 2.f;
                        v2 = (v + dv / 2.f);
                        u3 = u - du / 2.f;
                        v3 = (v + fDV);
                    }
                    break;
                    case 5:
                    {
                        result = bOverHi ? (0.f) : vecRad.x ;
                        p1 = Vector2(pos.x, pos.y);
                        p2 = Vector2(pos.x - (size.x / 2.f), pos.y - (size.y / 2.f));
                        p3 = Vector2(pos.x + result, pos.y - (size.y / 2.f));
                        float fPercent = result / size.x;
                        float fDU = du * fPercent;

                        u2 = u - du / 2.f;
                        v2 = (v - dv / 2.f);
                        u3 = u + fDU;
                        v3 = (v - dv / 2.f);
                    }
                    break;
                    default:
                        continue;
                }

                u1 = u;
                v1 = v;


                p1 = rs.transform.transform(p1);
                p2 = rs.transform.transform(p2);
                p3 = rs.transform.transform(p3);

                unsigned int rgba = getColor().rgba();

                fill_tex_coord(*pv, rgba, p1, u1, v1);
                pv++;
                fill_tex_coord(*pv, rgba, p2, u2, v2);
                pv++;
                fill_tex_coord(*pv, rgba, p3, u3, v3);
                pv++;
                fill_tex_coord(*pv, rgba, p2, u2, v2);
                pv++;



                renderer->addVertices(vertices, sizeof(vertices));
            }
        }

        //renderer->drawBatch();
    }

    std::string ProgressBar::dump(const dumpOptions& options) const
    {
        std::stringstream stream;
        stream << "{ProgressBar}\n";

        const char* dir = "dir_0";
        switch (_direction)
        {
            case dir_90:
                dir = "dir_90";
                break;
            case dir_180:
                dir = "dir_180";
                break;
            case dir_270:
                dir = "dir_270";
                break;
            case dir_radial_cw:
                dir = "dir_radial_cw";
                break;
            case __dir_radial_ccw:
                dir = "dir_radial_ccw";
                break;
            default:
                break;
        }
        stream << " direction=" << dir << "";

        stream << "\n" << inherited::dump(options);

        return stream.str();
    }
    void ProgressBar::setProgress(float f)
    {
        _progress = scalar::clamp(f, 0.0f, 1.0f);

        Event ev(PROGRESS_CHANGED);
        dispatchEvent(&ev);

        if (_direction == __dir_radial_ccw || _direction == dir_radial_cw)
            return;
        _update();
    }

    void ProgressBar::setDirection(direction dir)
    {
        _direction = dir;
        if (_direction == __dir_radial_ccw || _direction == dir_radial_cw)
        {
            _frame = _originalFrame;
            return;
        }
        _update();
    }

    float ProgressBar::getProgress() const
    {
        return _progress;
    }

    ProgressBar::direction ProgressBar::getDirection() const
    {
        return _direction;
    }

    void ProgressBar::serialize(serializedata* data)
    {
        inherited::serialize(data);
        pugi::xml_node node = data->node;
        data->node.set_name("ProgressBar");
        data->node.append_attribute("progress").set_value(_progress);
        data->node.append_attribute("direction").set_value((int)_direction);
    }

    void ProgressBar::deserialize(const deserializedata* data)
    {
        inherited::deserialize(data);
        _direction = (direction)data->node.attribute("direction").as_int();
        _progress = data->node.attribute("progress").as_float(1.0f);

        _update();
    }
}
