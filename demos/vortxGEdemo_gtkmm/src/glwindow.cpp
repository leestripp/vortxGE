#include <iostream>
#include <GL/gl.h>

#include "glwindow.h"
#include "demoscene.h"

// vortxGE
#include <timers/lsTimer.h>


glviewwindow::glviewwindow()
{
  demoscene = NULL;
  // keys
  key_a = false;
  key_d = false;
  key_w = false;
  key_s = false;
  //move
  delta = 0.1;
  move_speed = 1.0;
  // Mouse
  rot_speed = 0.05;
  move_speed = 0.01;
  rot_x = 0.0;
  rot_y = 0.0;

  buildUI();

  // keyboard handlers
  auto controller = Gtk::EventControllerKey::create();
  controller->signal_key_pressed().connect(sigc::mem_fun(*this, &glviewwindow::on_key_pressed), false);
  controller->signal_key_released().connect(sigc::mem_fun(*this, &glviewwindow::on_key_released), false);
  add_controller( controller );
}

glviewwindow::~glviewwindow()
{
}


bool glviewwindow::on_key_pressed(guint keyval, guint, Gdk::ModifierType state)
{
  // Modifiers
  // state & (Gdk::ModifierType::SHIFT_MASK | Gdk::ModifierType::CONTROL_MASK | Gdk::ModifierType::ALT_MASK)

  switch( keyval )
  {
    case GDK_KEY_w:
      // cout << "w pressed" << endl;
      key_w = true;
      break;

    case GDK_KEY_a:
      // cout << "a pressed" << endl;
      key_a = true;
      break;

    case GDK_KEY_s:
      // cout << "s pressed" << endl;
      key_s = true;
      break;

    case GDK_KEY_d:
      // cout << "d pressed" << endl;
      key_d = true;
      break;

    default:
      break;
  }
  return false;
}

void glviewwindow::on_key_released(guint keyval, guint, Gdk::ModifierType state)
{
  switch( keyval )
  {
    case GDK_KEY_w:
      key_w = false;
      break;

    case GDK_KEY_a:
      key_a = false;
      break;

    case GDK_KEY_s:
      key_s = false;
      break;

    case GDK_KEY_d:
      key_d = false;
      break;

    case GDK_KEY_F1:
      if( property_fullscreened() )
      {
        unfullscreen();

      } else
      {
        fullscreen();
      }
      break;

    case GDK_KEY_Escape:
      close();
      break;

    default:
      break;
  }
}


bool glviewwindow::Update()
{
  // cout << "Update called..." << endl;

  //m_GLArea.queue_draw();
  m_GLArea.queue_render();

  return true;
}

void glviewwindow::buildUI()
{
  set_title("vortxGEdemo_gtkmm");
  set_default_size(1920, 1080);

  //openGL settings
  m_GLArea.set_required_version( 4, 0 );
  m_GLArea.set_has_depth_buffer( true );
  m_GLArea.set_use_es( false );
  m_GLArea.set_hexpand(true);
  m_GLArea.set_vexpand(true);
  m_GLArea.set_auto_render(false);

  // Connect glarea signals
  m_GLArea.signal_realize().connect(sigc::mem_fun(*this, &glviewwindow::gla_realize));
  m_GLArea.signal_unrealize().connect(sigc::mem_fun(*this, &glviewwindow::gla_unrealize), false);
  m_GLArea.signal_render().connect(sigc::mem_fun(*this, &glviewwindow::gla_render), false);
  m_GLArea.signal_resize().connect(sigc::mem_fun(*this, &glviewwindow::gla_resize) );

  set_child(m_GLArea);
}

/* *************************
 * GLArea - realize
 * */
void glviewwindow::gla_realize()
{
  cout << "glviewwindow::gla_realize() - Called." << endl;

  m_GLArea.make_current();

  demoscene = new DemoScene();
  if(! demoscene  )
  {
    cout << "ERROR: glviewwindow::gla_realize() - Can't create demoscene instance." << endl;
     return;
  } else
  {
    cout << "glviewwindow::gla_realize() : demoscene instance created." << endl;
    if( demoscene->Init() )
    {
      cout << "glviewwindow::gla_realize() : demoscene->Init() OK." << endl;
      demoscene->Resize( 0, 0, m_GLArea.get_width(), m_GLArea.get_height() );

    } else
    {
      cout << "ERROR: glviewwindow::gla_realize() - demoscene->Init() Failed." << endl;
    }
  }

  // delta timer
  timer = new lsTimer();
  if(! timer  )
  {
    cout << "ERROR: glviewwindow::gla_realize() - Can't create timer instance." << endl;
     return;
  } else
  {
    timer->start_Timer();
    delta = 0.1;
  }

  // Update timer
  m_timer = Glib::signal_timeout().connect( sigc::mem_fun( *this, &glviewwindow::Update ), 1000 / 60 );
}

void glviewwindow::gla_unrealize()
{
  // Just to be safe.
  m_timer.disconnect();

  m_GLArea.make_current();

  // delete our test scene
  if( demoscene ) delete demoscene;
  if( timer ) delete timer;
}

void glviewwindow::gla_resize( int width, int height )
{
  // cout << "gla_resize called : " << width << "x" << height << endl;

  m_GLArea.make_current();

  if( demoscene )
  {
    demoscene->Resize( 0, 0, width, height );
  }
}


  // cout << "gla_render called..." << endl;

/* *************************
 * GLArea - Render
 * */
bool glviewwindow::gla_render(const Glib::RefPtr<Gdk::GLContext> &context )
{
  // cout << "gla_render called..." << endl;

  m_GLArea.make_current();

  if( demoscene )
  {
    // Move camera here so we can combine all key presses.
    demoscene->moveCamera( key_w, key_s, key_a, key_d, delta, move_speed );

    // Update
    demoscene->Update( delta );

    // Render the demo scene
    demoscene->Render();
  }

  // TODO: do we need this?
  m_GLArea.queue_draw();

  // Delta
  delta = timer->get_ElapsedTime();
  timer->start_Timer();

  return true;
}
