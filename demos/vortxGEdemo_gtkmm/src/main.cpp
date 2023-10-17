#include <gtkmm/application.h>

#include "glwindow.h"

int main(int argc, char* argv[])
{
  auto app = Gtk::Application::create("gtkmm.opengl.app");
  return app->make_window_and_run<glviewwindow>(argc, argv);
}
