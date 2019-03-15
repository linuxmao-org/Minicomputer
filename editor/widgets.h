#ifndef WIDGETS_H_
#define WIDGETS_H_

#include <Fl/Fl.H>
#include <math.h>

template <class W>
class Mw_Widget : public W
{
public:
    Mw_Widget(int x, int y, int w, int h, const char *l = 0)
        : W(x, y, w, h, l) {}

    int mouse_wheel_steps() const
    {
        return mouse_wheel_steps_;
    }

    void mouse_wheel_steps(int steps)
    {
        mouse_wheel_steps_ = steps;
    }

    int handle(int event) override
    {
        if (event == FL_MOUSEWHEEL) {
            int dy = Fl::event_dy();
            int steps = mouse_wheel_steps_;
            double wheelstep = (this->maximum() - this->minimum()) / steps;
            double normalstep = this->step();
            wheelstep = (fabs(wheelstep) > normalstep) ? wheelstep : copysign(normalstep, wheelstep);
            double old_value = this->value();
            double new_value = this->clamp(old_value - dy * wheelstep);
            if (new_value != old_value) {
                this->value(new_value);
                this->do_callback();
            }
            return 1;
        }
        return W::handle(event);
    }

private:
    int mouse_wheel_steps_ = 20;
};

#include <Fl/Fl_Roller.H>
typedef Mw_Widget<Fl_Roller> Mw_Roller;

#include <Fl/Fl_Dial.H>
typedef Mw_Widget<Fl_Dial> Mw_Dial;

#endif // WIDGETS_H_
