#include <iostream>

#include "video_processor.hpp"

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    std::cout << "Usage: " << argv[0] << " FILENAME" << std::endl;
  }
  else
  {
    Gst::init(argc, argv);

    VideoProcessor video(argv[1]);
    while(video.is_playing())
    {
      //std::cout << "my loop" << std::endl;
      video.update();
    }
  }

  return 0;
}

/* EOF */
