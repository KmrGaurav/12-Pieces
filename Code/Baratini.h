#ifndef _BARATINI_H
#define _BARATINI_H

struct extra_info
{
    s32 MarginWidth;
    s32 MarginHeight;
    s32 SquareLength;
};
struct game_offscreen_buffer
{
    void* Memory;
    s32   Width;
    s32   Height;
    s32   Pitch;
};

#pragma pack(push, 1)
struct bitmap_header
{
  u16 FileType;
  u32 FileSize;
  u16 Reserved1;
  u16 Reserved2;
  u32 BitmapOffset;
  u32 Size;
  s32 Width;
  s32 Height;
  u16 Planes;
  u16 BitsPerPixel;
  u32 Compression;
  u32 SizeOfBitmap;
  s32 HorzResolution;
  s32 VertResolution;
  u32 ColorsUsed;
  u32 ColorsImportant;
};
#pragma pack(pop)

struct game_asset
{
    s32   Width;
    s32   Height;
    u16   BitsPerPixel;
    s32   BitmapSize;
    s32   Pitch;
    void* Pixels;
    s32   X;
    s32   Y;
};

enum player
{
    NoPlayer,
    Player1,
    Player2
};

enum is_alive
{
    Alive,
    Dead
};

struct player_info
{
    enum player Player;/*Player1 or Player2 or No Player*/
    s32 Index;
    s32 X;
    s32 Y;
    bool Highlight;
    enum is_alive IsAlive;/*No need when using Linked List*/
    game_asset* Asset;
};

struct position
{
    s32 Index;
    s32 X;
    s32 Y;
    player_info* Player;
};

struct point
{
    s32 X;
    s32 Y;
};

struct double_point
{
    s32 X1;
    s32 Y1;
    s32 X2;
    s32 Y2;
};

enum whos_turn
{
    MyTurn,
    YourTurn
};

#endif // _BARATINI_H
