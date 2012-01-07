#include <windows.h>


//-----------------------------------------------------------------
// prototype of the main function
int main(int argc, char** argv);

//-----------------------------------------------------------------
// stub WinMain, simply calls main
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    return main(__argc, __argv);
}
