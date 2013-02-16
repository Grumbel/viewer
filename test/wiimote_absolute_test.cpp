#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wshadow"

#include <glm/glm.hpp>
#include <unistd.h>
#include <wiicpp.h>
#include <chrono>

class WiimoteOrientation
{
public:
  glm::vec3 orient;

  WiimoteOrientation()
  {
  }
};


glm::vec3 g_orientation;

std::chrono::high_resolution_clock::time_point g_last_time;
int g_event_count = 0;
int g_duration_count = 0;

void handle_event(CWiimote& wiimote)
{
  char prefixString[64];
  sprintf(prefixString, "Controller [%i]: ", wiimote.GetID());

  if(wiimote.Buttons.isJustPressed(CButtons::BUTTON_UP))
  {
    g_orientation = glm::vec3();
  }

  if(wiimote.Buttons.isJustPressed(CButtons::BUTTON_RIGHT))
  {
    wiimote.EnableMotionPlus(CWiimote::ON);
    wiimote.ExpansionDevice.MotionPlus.Gyroscope.SetGyroThreshold(0);
  }

  if(wiimote.Buttons.isJustPressed(CButtons::BUTTON_LEFT))
  {
    wiimote.EnableMotionPlus(CWiimote::OFF);
  }

  if(wiimote.isUsingACC())
  {
    //float pitch, roll, yaw;
    glm::vec3 rate;
    wiimote.Accelerometer.GetOrientation(rate.x, rate.y, rate.z);
    printf("%s wiimote roll  = %f\n", prefixString, rate.x);
    printf("%s wiimote pitch = %f\n", prefixString, rate.y);
    printf("%s wiimote yaw   = %f\n", prefixString, rate.z);
  }

  // if the Motion Plus is turned on then print angles
  if(wiimote.isUsingMotionPlus()) 
  {
    if (g_last_time == std::chrono::high_resolution_clock::time_point())
    {
      g_last_time = std::chrono::high_resolution_clock::now();
    }

    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_last_time).count();
    g_event_count += 1;
    g_duration_count += static_cast<int>(duration);

    glm::vec3 rate;
    wiimote.ExpansionDevice.MotionPlus.Gyroscope.GetRates(rate.y, rate.x, rate.z);
    printf("motion plus - pitch: %8.2f roll: %8.2f yaw: %8.2f   pitch: %8.2f roll: %8.2f yaw: %8.2f - %8d %8d %8f\n",
           rate.x, rate.y, rate.z, 
           g_orientation.x, g_orientation.y, g_orientation.z,
           static_cast<int>(duration),
           static_cast<int>(g_duration_count),
           static_cast<float>(g_duration_count) / g_event_count);
    g_orientation += 0.01f * rate;

    g_last_time = now;
  }

  if(wiimote.isUsingIR())
  {
    std::vector<CIRDot>::iterator i;
    int x, y;
    int index;

    printf("%s Num IR Dots: %i\n", prefixString, wiimote.IR.GetNumDots());
    printf("%s IR State: %u\n", prefixString, wiimote.IR.GetState());

    std::vector<CIRDot>& dots = wiimote.IR.GetDots();

    for(index = 0, i = dots.begin(); i != dots.end(); ++index, ++i)
    {
      if((*i).isVisible())
      {
        (*i).GetCoordinate(x, y);
        printf("%s IR source %i: (%i, %i)\n", prefixString, index, x, y);

        wiimote.IR.GetCursorPosition(x, y);
        printf("%s IR cursor: (%i, %i)\n", prefixString, x, y);
        printf("%s IR z distance: %f\n", prefixString, wiimote.IR.GetDistance());
      }
    }
  }
}

int main(int argc, char** argv)
{
  CWii wii(1);

  int reloadWiimotes = 0;
  int index;
	
  // Find and connect to the wiimotes
  std::vector<CWiimote>& wiimotes = wii.FindAndConnect();

  if (!wiimotes.size())
  {
    cout << "No wiimotes found." << endl;
    return 0;
  }
  else
  {
    // Setup the wiimotes
    for(auto& wiimote : wiimotes)
    {
      wiimote.SetLEDs(CWiimote::LED_1);
      //wiimote.IR.SetMode(CIR::ON);
      //wiimote.SetMotionSensingMode(CWiimote::ON);
      //wiimote.EnableMotionPlus(CWiimote::ON);
    }

    do
    {
      if(reloadWiimotes)
      {
        // Regenerate the list of wiimotes
        wiimotes = wii.GetWiimotes();
        reloadWiimotes = 0;
      }

      //Poll the wiimotes to get the status like pitch or roll
      if(wii.Poll())
      {
        for(auto& wiimote : wiimotes)
        {
          // Use a reference to make working with the iterator handy.
          switch(wiimote.GetEvent())
          {

            case CWiimote::EVENT_EVENT:
              handle_event(wiimote);
              break;

            case CWiimote::EVENT_STATUS:
              //HandleStatus(wiimote);
              break;

            case CWiimote::EVENT_DISCONNECT:
            case CWiimote::EVENT_UNEXPECTED_DISCONNECT:
              //HandleDisconnect(wiimote);
              reloadWiimotes = 1;
              break;

            case CWiimote::EVENT_READ_DATA:
              //HandleReadData(wiimote);
              break;

            case CWiimote::EVENT_NUNCHUK_INSERTED:
              //HandleNunchukInserted(wiimote);
              reloadWiimotes = 1;
              break;

            case CWiimote::EVENT_CLASSIC_CTRL_INSERTED:
              //HandleClassicInserted(wiimote);
              reloadWiimotes = 1;
              break;

            case CWiimote::EVENT_GUITAR_HERO_3_CTRL_INSERTED:
              //HandleGH3Inserted(wiimote);
              reloadWiimotes = 1;
              break;

            case CWiimote::EVENT_MOTION_PLUS_INSERTED:
              //wiimote.EnableMotionPlus(CWiimote::ON);
              cout << "Motion Plus inserted." << endl;
              break;
								
            case CWiimote::EVENT_BALANCE_BOARD_INSERTED:
              cout << "Balance Board connected.\n"  << endl;
              break;
						
            case CWiimote::EVENT_BALANCE_BOARD_REMOVED:
              cout << "Balance Board disconnected.\n"  << endl;
              break;
						
            case CWiimote::EVENT_NUNCHUK_REMOVED:
            case CWiimote::EVENT_CLASSIC_CTRL_REMOVED:
            case CWiimote::EVENT_GUITAR_HERO_3_CTRL_REMOVED:
            case CWiimote::EVENT_MOTION_PLUS_REMOVED:
              cout << "An expansion was removed." << endl;
              //HandleStatus(wiimote);
              reloadWiimotes = 1;
              break;

            default:
              break;
          }
        }
      }

    } while(wiimotes.size()); // Go so long as there are wiimotes left to poll

    return 0;
  }
}

/* EOF */
