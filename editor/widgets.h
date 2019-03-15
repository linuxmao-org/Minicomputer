#ifndef WIDGETS_H_
#define WIDGETS_H_

#include <Fl/Fl_Roller.H>
#include <Fl/Fl.H>

class Mw_Roller : public Fl_Roller
{
public:
    Mw_Roller(int x, int y, int w, int h, const char *l = 0)
        : Fl_Roller(x, y, w, h, l) {}

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
            wheelstep = (wheelstep > normalstep) ? wheelstep : normalstep;
            double old_value = this->value();
            double new_value = this->clamp(old_value - dy * wheelstep);
            if (new_value != old_value) {
                this->value(new_value);
                this->do_callback();
            }
            return 1;
        }
        return Fl_Roller::handle(event);
    }

private:
    int mouse_wheel_steps_ = 20;
};

#endif // WIDGETS_H_
