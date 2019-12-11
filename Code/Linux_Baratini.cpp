#include <X11/Xlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <malloc.h>
#include <stdint.h>

#define internal        static
#define global_variable static
#define local_persist   static

typedef  int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;

typedef  uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

#include "Baratini.h"
#include "Baratini.cpp"

internal bool
LoadAsset(game_asset* Asset, char* FileName)
{
    FILE *InFile;
    InFile = fopen(FileName, "rb");
    if(InFile)
    {
        bitmap_header Header = {};
        fread(&Header, sizeof(Header), 1, InFile);

        Asset->Width        = Header.Width;
        Asset->Height       = Header.Height;
        Asset->BitsPerPixel = Header.BitsPerPixel;
        Asset->BitmapSize   = Asset->BitsPerPixel*Asset->Width*Asset->Height/8;
        Asset->Pitch        = Asset->BitsPerPixel*Asset->Width/8;
        Asset->Pixels       = (u32*)malloc(Asset->BitmapSize);
        
        fread(Asset->Pixels, Asset->BitmapSize, 1, InFile);
        
        fclose(InFile);
        return true;
    }
    else
    {
        printf("Asset isn\'t loaded\n");
        return false;
    }
}

struct linux_window_info
{
    Display*             BaratiniDisplay;
    s32                  Screen;
    Window               RootWindow;
    s32                  Depth;
    Visual*              BaratiniVisual;
    GC                   GraphicsContext;
    Window               BaratiniWindow;
    XSetWindowAttributes Attributes;
};

struct linux_offscreen_buffer
{
    XImage* Image;
    void*   Memory;
    int     Width;
    int     Height;
    int     Pitch;
};

global_variable linux_window_info GlobalWindowinfo;

global_variable bool GlobalRunning;
global_variable linux_offscreen_buffer GlobalBackbuffer;
global_variable enum whos_turn Turn;
global_variable extra_info Extra;
global_variable position Positions[25];
global_variable player_info Players[24];

struct linux_window_dimension
{
    int Width;
    int Height;
};

linux_window_dimension
LinuxGetWindowDimension(linux_window_info* Info)
{
/*  // This calculates physical screen size  
    linux_window_dimension Result;
    Result.Width  = DisplayWidth (Info->BaratiniDisplay, Info->Screen);
    Result.Height = DisplayHeight(Info->BaratiniDisplay, Info->Screen);
    printf("width = %d and height = %d\n", Result.Width, Result.Height);
    return Result;*/
    
    XWindowAttributes Attributes;
    XGetWindowAttributes(Info->BaratiniDisplay, Info->BaratiniWindow, &Attributes);

    linux_window_dimension Result;
    Result.Width  = Attributes.width;
    Result.Height = Attributes.height;
    return Result;
}

internal void
LinuxResizeDIBSection(linux_window_info Info,
                      linux_offscreen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        munmap(Buffer->Memory, Width*Height*4);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    int BytesPerPixel = 4;
    Buffer->Pitch = Width*BytesPerPixel;

    int BitmapMemorySize = Width*Height*BytesPerPixel;
    Buffer->Memory = mmap(0, BitmapMemorySize, PROT_READ | PROT_WRITE,
                          MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    Buffer->Image = XCreateImage(Info.BaratiniDisplay,
                                 Info.BaratiniVisual, 
                                 24, ZPixmap, 0,
                                 (char*)Buffer->Memory,
                                 Width, Height, 32, 0);
}

internal void
LinuxDisplayBufferInWindow(linux_offscreen_buffer* Buffer,
                           int Width, int Height)
{
    
    XPutImage(GlobalWindowinfo.BaratiniDisplay,
              GlobalWindowinfo.BaratiniWindow,
              GlobalWindowinfo.GraphicsContext,
              Buffer->Image, 0, 0, 0, 0, Width, Height);
}

internal void
LinuxMainWindowEvent(XEvent* Event)
{
    local_persist u32 Index;
    local_persist bool Select;
    local_persist point MousePointer;
    local_persist point DeltaPointer;
    local_persist u32 DestinationIndex;
    switch(Event->type)
    {
    case Expose:
    {
        linux_window_dimension Dimension = LinuxGetWindowDimension(&GlobalWindowinfo);
        LinuxDisplayBufferInWindow(&GlobalBackbuffer, Dimension.Width, Dimension.Height);
    }
    break;
    case KeyPress:
    {
        GlobalRunning = false;
    }
    break;
    case MotionNotify:
    {
        MousePointer.X = Event->xbutton.x;
        MousePointer.Y = Event->xbutton.y;
        if(!Select)
        {
            if(IsPointerOnTile(&MousePointer, Positions, &Extra, &Turn, &Index))
            {
                HighlightTile(&MousePointer, Positions, &Extra, &Turn);
                if(Event->xbutton.state & Button1Mask)
                {
                    Select = true;
                }
            }            
        }
        else
        {
            if(Event->xbutton.state & Button1Mask)
            {
                SlidePlayer(&MousePointer, Positions, &Index, &DeltaPointer);
            }
            else
            {
                
            }
        }
    }
    break;
    case ButtonPress:
    {
        
    }
    break;
    case ButtonRelease:
    {
        if(Select)//This condition is necessary otherwise it crashes.
        {
            Select = false;
            bool ShouldKill = false;
            u32 DestroyedIndex;
            if(ShouldPlayerMove(Positions, &Index, &DeltaPointer, 
                                &Extra, &DestinationIndex, &ShouldKill, &DestroyedIndex))
            {
                if(ShouldKill)
                {
                    KillPlayer(Positions, Players, &Index, &DestinationIndex, &DestroyedIndex);
                }
                else
                {
                    MovePlayer(Positions, &Index, &DestinationIndex);
                }
                ToggleTurn(&Turn);
            }
            else
            {
                Positions[Index].Player->X = Positions[Index].X;
                Positions[Index].Player->Y = Positions[Index].Y;
            }
        }
    }
    break;
    case ClientMessage:
    {
        /*if(Event.xclient.data.l[0] == (long)wmDeleteWindowMessage)
        {
            GlobalRunning = false;
        }*/
    }
    break;
    }
}

int
main(int ArgumentCount, char** ArgumentValues)
{
    game_asset PlayerAsset1, PlayerAsset2;
    LoadAsset(&PlayerAsset1, (char*)"Player1.bmp");
    LoadAsset(&PlayerAsset2, (char*)"Player2.bmp");

    GlobalWindowinfo.BaratiniDisplay    = XOpenDisplay(NULL);
    GlobalWindowinfo.Screen             = DefaultScreen(GlobalWindowinfo.BaratiniDisplay);
    GlobalWindowinfo.RootWindow         = XRootWindow(GlobalWindowinfo.BaratiniDisplay, GlobalWindowinfo.Screen);
    GlobalWindowinfo.Depth              = DefaultDepth(GlobalWindowinfo.BaratiniDisplay, GlobalWindowinfo.Screen);
    GlobalWindowinfo.BaratiniVisual     = DefaultVisual(GlobalWindowinfo.BaratiniDisplay, GlobalWindowinfo.Screen);
    GlobalWindowinfo.GraphicsContext    = DefaultGC(GlobalWindowinfo.BaratiniDisplay, 0);
    GlobalWindowinfo.Attributes.background_pixel = XWhitePixel(GlobalWindowinfo.BaratiniDisplay, GlobalWindowinfo.Screen);

    LinuxResizeDIBSection(GlobalWindowinfo, &GlobalBackbuffer, 1080, 650);
    
    GlobalWindowinfo.BaratiniWindow     = XCreateWindow(GlobalWindowinfo.BaratiniDisplay, GlobalWindowinfo.RootWindow,
                            0, 0, 1080, 650, 1,
                            GlobalWindowinfo.Depth, InputOutput, GlobalWindowinfo.BaratiniVisual,
                               CWBackPixel, &GlobalWindowinfo.Attributes);
    XSelectInput(GlobalWindowinfo.BaratiniDisplay, GlobalWindowinfo.BaratiniWindow, ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | Button1MotionMask | PointerMotionMask);
    XMapWindow(GlobalWindowinfo.BaratiniDisplay, GlobalWindowinfo.BaratiniWindow);
    XFlush(GlobalWindowinfo.BaratiniDisplay);
    Atom WM_QUIT = XInternAtom(GlobalWindowinfo.BaratiniDisplay, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(GlobalWindowinfo.BaratiniDisplay, GlobalWindowinfo.BaratiniWindow, &WM_QUIT, 1);

    
    linux_window_dimension Dim = LinuxGetWindowDimension(&GlobalWindowinfo);
    
    Extra.SquareLength = 150;
    Extra.MarginWidth  = (Dim.Width  - 4*Extra.SquareLength)/2;
    Extra.MarginHeight = (Dim.Height - 4*Extra.SquareLength)/2;
    
    PlayersInitialization(Players, &Extra,
                          &PlayerAsset1, &PlayerAsset2);

    InitializePositions(Positions, &Extra, Players);

    GlobalRunning = true;
    while(GlobalRunning)
    {
        XEvent Event;
        while(XPending(GlobalWindowinfo.BaratiniDisplay) > 0)
        {
            XNextEvent(GlobalWindowinfo.BaratiniDisplay, &Event);
            if(Event.xclient.data.l[0] == (long)WM_QUIT)
            {
                GlobalRunning = false;
            }
            LinuxMainWindowEvent(&Event);
        }
        
        game_offscreen_buffer Buffer = {};
        Buffer.Memory = GlobalBackbuffer.Memory;
        Buffer.Width  = GlobalBackbuffer.Width;
        Buffer.Height = GlobalBackbuffer.Height;
        Buffer.Pitch  = GlobalBackbuffer.Pitch;

        GameUpdateAndRender(&Buffer, &Extra, Positions,
                            Players, &PlayerAsset1, &PlayerAsset2);

        linux_window_dimension Dimension = LinuxGetWindowDimension(&GlobalWindowinfo);
        LinuxDisplayBufferInWindow(&GlobalBackbuffer,
                                   Dimension.Width, Dimension.Height);
    }
    return 0;
}

