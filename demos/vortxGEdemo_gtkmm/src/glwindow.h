#include <gtkmm/window.h>
#include <gtkmm/glarea.h>

#include <gtkmm.h>
#include <glibmm.h>

class DemoScene;
class lsTimer;

using namespace std;


class glviewwindow : public Gtk::Window
{
public:
    glviewwindow();
    virtual ~glviewwindow();

    void buildUI();

private:
// Slots
    void gla_realize();
    void gla_unrealize();
    void gla_resize( int width, int height );
    bool gla_render(const Glib::RefPtr<Gdk::GLContext> &context);
    // keyboard
    bool on_key_pressed(guint keyval, guint, Gdk::ModifierType state);
    void on_key_released(guint keyval, guint, Gdk::ModifierType state);
    // timer
    bool Update(); // force a render of the openGL area.

// Gtkmm OpenGL
    Gtk::GLArea m_GLArea;

// Glibmm timer
    sigc::connection m_timer;

    DemoScene	*demoscene;
    double      delta;
    lsTimer     *timer;

    // keys
    bool key_a;
    bool key_d;
    bool key_w;
    bool key_s;
    // Mouse
    double rot_speed;
    double move_speed;
    double rot_x;
    double rot_y;
};
