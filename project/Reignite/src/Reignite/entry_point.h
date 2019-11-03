#ifndef _ENTRY_POINT_
#define _ENTRY_POINT_ 1

#ifdef RI_PLATFORM_WINDOWS

extern Reignite::Application* Reignite::CreateApplication();

int main(int argc, char** argv) {

  Reignite::CreateApplication()->Run();
}

#endif // RI_PLATFORM_WINDOWS

#endif // _ENTRY_POINT_
