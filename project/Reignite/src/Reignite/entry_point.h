#ifndef _RI_ENTRY_POINT_
#define _RI_ENTRY_POINT_ 1

#ifdef RI_PLATFORM_WINDOWS

extern Reignite::Application* Reignite::CreateApplication();

int main(int argc, char** argv) {

  Reignite::Log::Init(); // TODO: This should be changed to application source code
  Reignite::CreateApplication()->Run();
}

#endif // RI_PLATFORM_WINDOWS

#endif // _RI_ENTRY_POINT_
