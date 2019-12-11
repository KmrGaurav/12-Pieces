#include <windows.h>
#include <stdint.h>
#include <stdio.h>

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
        Asset->Pitch        = Asset->BitsPerPixel*(Asset->Width)/8;
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

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void*      Memory;
    int        Width;
    int        Height;
    int        Pitch;
};

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackbuffer;
global_variable enum whos_turn Turn=YourTurn;
global_variable extra_info Extra;
global_variable position Positions[25];
global_variable player_info Players[24];

struct win32_window_dimension
{
    int Width;
    int Height;
};

win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    int BytesPerPixel = 4;
    
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = Buffer->Width*Buffer->Height*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    
    Buffer->Pitch = Width*BytesPerPixel;
}

internal void
Win32DisplayBufferInWindow(HDC DeviceContext,
			   int WindowWidth, int WindowHeight,
			   win32_offscreen_buffer* Buffer)
{
    StretchDIBits(DeviceContext,
                  0, 0, WindowWidth, WindowHeight,
                  0, 0, Buffer->Width, Buffer->Height,
                  Buffer->Memory,
                  &Buffer->Info,
                  DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK
Win32MainWindowCallback(HWND Window,
                        UINT Message,
                        WPARAM WParam,
                        LPARAM LParam)
{
    LRESULT Result = 0;
    local_persist u32 Index;
    local_persist bool Select;
    local_persist point MousePointer;
    local_persist point DeltaPointer;
    local_persist u32 DestinationIndex;
    switch(Message)
    {
    case WM_SIZE:
    {
    } break;
    case WM_MOUSEMOVE:
    {        
        MousePointer.X = LOWORD(LParam);
        MousePointer.Y = HIWORD(LParam);
        if(!Select)
        {
            if(IsPointerOnTile(&MousePointer, Positions, &Extra, &Turn, &Index))
            {
                HighlightTile(&MousePointer, Positions, &Extra, &Turn);
                if(WParam & MK_LBUTTON)
                {
                    Select = true;
                }
            }            
        }
        else
        {
            if(WParam & MK_LBUTTON)
            {
                SlidePlayer(&MousePointer, Positions, &Index, &DeltaPointer);
            }
            else
            {

            }
        }
    }
    break;
    case WM_LBUTTONDOWN:
    {
        
    }
    break;
    case WM_LBUTTONUP:
    {
        if(Select)//This condition is necessary otherwise game crashes.
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
    case WM_CLOSE:
    {
        GlobalRunning = false;
    } 
    break;
    case WM_ACTIVATEAPP:
    {
        
    } break;
    case WM_DESTROY:
    {
        GlobalRunning = false;
    }
    break;    
    case WM_PAINT:
    {
        PAINTSTRUCT Paint;
        HDC DeviceContext = BeginPaint(Window, &Paint);
        
        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, 
                                   Dimension.Height, &GlobalBackbuffer);
        EndPaint(Window, &Paint);     
    } 
    break;
    default:
    {
        Result = DefWindowProc(Window, Message, WParam, LParam);
    } 
    break;
    }

    return Result;
}

int CALLBACK
WinMain (HINSTANCE Instance,
         HINSTANCE PrevInstance,
         LPSTR CommandLine,
         int ShowCode)
{
    game_asset PlayerAsset1, PlayerAsset2;
    LoadAsset(&PlayerAsset1, (char*)"Player1.bmp");
    LoadAsset(&PlayerAsset2, (char*)"Player2.bmp");    

    WNDCLASS WindowClass = {};
    Win32ResizeDIBSection(&GlobalBackbuffer, 1080, 650);
    
    WindowClass.style         = CS_HREDRAW|CS_VREDRAW;
    WindowClass.lpfnWndProc   = Win32MainWindowCallback;
    WindowClass.hInstance     = Instance;
    WindowClass.hCursor       = LoadCursor(NULL, IDC_ARROW);
    WindowClass.lpszClassName = "Baratini";

    if(RegisterClass(&WindowClass))
    {

        HWND Window = CreateWindowEx(0,
                                     WindowClass.lpszClassName,
                                     "Baratini",
                                     WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                     CW_USEDEFAULT, CW_USEDEFAULT,
                                     1096, 689,
                                     0, 0, Instance, 0);
        if(Window)
        {
            win32_window_dimension Dim  = Win32GetWindowDimension(Window);
       
            Extra.SquareLength = 150;
            Extra.MarginWidth  = (Dim.Width  - 4*Extra.SquareLength)/2;
            Extra.MarginHeight = (Dim.Height - 4*Extra.SquareLength)/2;
            
            PlayersInitialization(Players, &Extra,
                                  &PlayerAsset1, &PlayerAsset2);

            InitializePositions(Positions, &Extra, Players);
 
                
            GlobalRunning = true;
            while(GlobalRunning)
            {
                MSG Message;
                while(PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
                {
                    if(Message.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    TranslateMessage(&Message);
                    DispatchMessage(&Message);
                }
                
                game_offscreen_buffer Buffer = {};
                Buffer.Memory = GlobalBackbuffer.Memory;
                Buffer.Width  = GlobalBackbuffer.Width;
                Buffer.Height = GlobalBackbuffer.Height;
                Buffer.Pitch  = GlobalBackbuffer.Pitch;
         
                GameUpdateAndRender(&Buffer, &Extra, Positions,
                                    Players, &PlayerAsset1, &PlayerAsset2);

                HDC DeviceContext = GetDC(Window);

                win32_window_dimension Dimension = Win32GetWindowDimension(Window);
                Win32DisplayBufferInWindow(DeviceContext, Dimension.Width, 
                                   Dimension.Height, &GlobalBackbuffer);
                
                ReleaseDC(Window, DeviceContext);
            }
        }
    }
    return 0;
}

