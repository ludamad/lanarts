#ifndef GLGRIDATLAS_H
#define GLGRIDATLAS_H

#include "GLImage.h"
#include <memory>
#include <lcommon/smartptr.h>
#include <lcommon/fatal_error.h>


class GLGridAtlas {
public:
    GLGridAtlas(int tilew, int tileh, int n_rows, int n_cols) {
        _tilew = tilew;
        _tileh = tileh;
        _n_rows = n_rows;
        _n_cols = n_cols;
    }

    void allocate(const oxygine::ImageData& data, const BBoxF& region,
                  smartptr<GLImage>& image_out, BBoxF& region_out) {
        if (!_buffer) {
            _buffer.set(new GLImage(Size(_tilew * _n_rows, _tileh * _n_cols)));
            printf("NEW BUFFER AHOY\n");
            _i_x = 0;
            _i_y = 0;
        }
        region_out = BBoxF(_i_x * _tilew, _i_y * _tileh, (_i_x+1) * _tilew, (_i_y+1) * _tileh);
        image_out = _buffer;
        _buffer->texture_->updateRegion(region_out.x1, region_out.y1,
                                        data.getRect(region.x1, region.y1, region.width(), region.height()));
        // Navigate to next
        _i_x++;
        if (_i_x >= _n_rows) {
            _i_x = 0;
            _i_y++;
            if (_i_y >= _n_cols) {
                _buffer.clear();
            }
        }
    }
    void allocate(const std::string& filename, BBoxF region,
                  smartptr<GLImage>& image_out, BBoxF& region_out) {
        using namespace oxygine;

        file::buffer bf;
        file::read(filename, bf);
        Image mt;
        if (!mt.init(bf, true)) {
            printf("Texture from image '%s' could not be loaded\n",
                   filename.c_str());
            fatal_error(); // Don't fatal error for now! TODO conditional on environment variable?
        }
        if (region == BBoxF()) {
            region = BBoxF(0,0, mt.getWidth(), mt.getHeight());
        }
        allocate(mt.lock(), region, image_out, region_out);
    }

    smartptr<GLImage> _buffer;
    int _tilew, _tileh, _n_rows, _n_cols;
    int _i_x = 0, _i_y = 0;
};


#endif // GLGRIDATLAS_H
