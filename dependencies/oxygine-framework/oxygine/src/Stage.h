#pragma once
#include "oxygine-include.h"
#include "Actor.h"


namespace oxygine
{
    DECLARE_SMART(Stage, spStage);
    class Stage : public Actor
    {
        INHERITED(Actor);
    public:
        enum
        {
            ACTIVATE = sysEventID('S', 'A', 'c'),
            DEACTIVATE = sysEventID('S', 'D', 'c'),
            LOST_CONTEXT = sysEventID('S', 'L', 'C'),
        };

        static spStage instance;
        static Rect calcCenteredViewport(const Point& displaySize, const Point& gameSize);

        Stage(bool autoHandleReset = false);
        ~Stage();

        /**Initializes size, scale and position of Stage
        @param is real display size of device.
        @param is your "virtual" preferred size. You could change it to any size you need
        */
        void init(const Point& displaySize, const Point& gameSize);

        /**use Scissor test for outer Stage area*/
        void setClipOuterArea(bool clip) {_clipOuter = clip;}
        /**Render all actors*/
        void render(const Color* clearColor, const Rect& viewport, const Matrix& view, const Matrix& proj);
        /**Render all actors, simplified version*/
        void render(const Color& clearColor, const Rect& viewport);

        //const Rect    &getArea();

        /**Updates each children*/
        void update();

        void cleanup();

        RectF getDestRect() const override;


        std::string dump(const dumpOptions& opt) const override;

#if OXYGINE_SDL
        void associateWithWindow(SDL_Window* wnd);
        SDL_Window* getAssociatedWindow() const;
        static Stage* getStageFromWindow(SDL_Window* wnd);
#endif

    protected:
        void onDeactivate(Event*);
        void onActivate(Event*);

        //bool _active;

        bool isOn(const Vector2& localPosition, float localScale) override;

    private:

        timeMS _statUpdate;
        bool    _clipOuter;
        Rect    _viewport;

#if OXYGINE_SDL
        SDL_Window* _window;
#endif // OXYGINE_SDL


        friend class DebugActor;
    };

    inline const spStage& getStage() {return Stage::instance;}
}
