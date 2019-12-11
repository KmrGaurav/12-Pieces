internal void
GameBackground(game_offscreen_buffer* Buffer)
{
    int* Out = (int*)Buffer->Memory;
    for(int Y = 0; Y < Buffer->Height; Y++)
    {
        for(int X = 0; X < Buffer->Width; X++)
        {
            *(Out + X) = X*255/Buffer->Width | Y*255/Buffer->Height<<8 | (255 - X*255/Buffer->Width)<<16;
        }
        Out += Buffer->Pitch/4;
    }
}

internal s32
Mod(s32 number)
{
    return number > 0 ? number : -number;
}

internal void
PlotLowLine(game_offscreen_buffer* Buffer, s32 x1, s32 y1, s32 x2, s32 y2, s32 Color)
{
    s32 dx = x2 - x1;
    s32 dy = y2 - y1;
    s32 yi = 1;
    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }
    s32 D = 2*dy - dx;
    s32 y = y1;

    for(s32 x = x1; x < x2; x++)
    {
        *((int*)(Buffer->Memory) + (y - 2)*Buffer->Width + x) = Color;
        *((int*)(Buffer->Memory) + (y - 1)*Buffer->Width + x) = Color;
        *((int*)(Buffer->Memory) +       y*Buffer->Width + x) = Color;
        *((int*)(Buffer->Memory) + (y + 1)*Buffer->Width + x) = Color;
        *((int*)(Buffer->Memory) + (y + 2)*Buffer->Width + x) = Color;
        if (D > 0)
        {
           y = y + yi;
           D = D - 2*dx;
        }
        D = D + 2*dy;
    }
}

internal void
PlotHighLine(game_offscreen_buffer* Buffer, s32 x1, s32 y1, s32 x2, s32 y2, s32 Color)
{
    s32 dx = x2 - x1;
    s32 dy = y2 - y1;
    s32 xi = 1;
    if (dx < 0)
    {
        xi = -1;
        dx = -dx;
    }
    s32 D = 2*dx - dy;
    s32 x = x1;

    for(s32 y = y1; y < y2; y++)
    {
        *((int*)(Buffer->Memory) + x - 2 + y*Buffer->Width) = Color;
        *((int*)(Buffer->Memory) + x - 1 + y*Buffer->Width) = Color;
        *((int*)(Buffer->Memory) + x     + y*Buffer->Width) = Color;
        *((int*)(Buffer->Memory) + x + 1 + y*Buffer->Width) = Color;
        *((int*)(Buffer->Memory) + x + 2 + y*Buffer->Width) = Color;
        if (D > 0)
        {
            x = x + xi;
            D = D - 2*dy;
        }
        D = D + 2*dx;
    }
}

internal void
Line(game_offscreen_buffer* Buffer, s32 x1, s32 y1, s32 x2, s32 y2, s32 Color)
{
    if (Mod(y2 - y1) < Mod(x2 - x1))
    {
        if (x1 > x2)
            PlotLowLine(Buffer, x2, y2, x1, y1, Color);
        else
            PlotLowLine(Buffer, x1, y1, x2, y2, Color);
    }
    else
    {
        if (y1 > y2)
            PlotHighLine(Buffer, x2, y2, x1, y1, Color);
        else
            PlotHighLine(Buffer, x1, y1, x2, y2, Color);
    }
}

internal void
GamePattern(game_offscreen_buffer *Buffer, extra_info* Extra)
{
    s32 Color = 0xffffffff;
    u32 MarginWidth  = Extra->MarginWidth;
    u32 MarginHeight = Extra->MarginHeight;
    u32 Length       = Extra->SquareLength;

    for(u32 A = 0; A <= 4*Length; A += Length)
    {
        Line(Buffer, MarginWidth, MarginHeight + A,
             MarginWidth + 4*Length, MarginHeight + A, Color);
        Line(Buffer, MarginWidth + A, MarginHeight,
             MarginWidth + A,  MarginHeight + 4*Length, Color);
    }

    Line(Buffer, MarginWidth, MarginHeight,
         MarginWidth + 4*Length, MarginHeight + 4*Length, Color);
    Line(Buffer, MarginWidth + 4*Length, MarginHeight,
         MarginWidth, MarginHeight + 4*Length, Color);

    Line(Buffer, MarginWidth + 2*Length,
         MarginHeight, MarginWidth, MarginHeight + 2*Length, Color);
    Line(Buffer, MarginWidth + 2*Length,
         MarginHeight, MarginWidth + 4*Length, MarginHeight + 2*Length, Color);
    Line(Buffer, MarginWidth, MarginHeight + 2*Length,
         MarginWidth + 2*Length, MarginHeight + 4*Length, Color);
    Line(Buffer, MarginWidth + 4*Length, MarginHeight + 2*Length,
         MarginWidth + 2*Length, MarginHeight + 4*Length, Color);
}

internal void
DrawRectangle(game_offscreen_buffer* Buffer, s32 x1, s32 y1, s32 x2, s32 y2, u32 Color)
{
    Line(Buffer, x1, y1, x2, y1, Color);
    Line(Buffer, x1, y1, x1, y2, Color);
    Line(Buffer, x2, y1, x2, y2, Color);
    Line(Buffer, x1, y2, x2, y2, Color);
}

internal void
RenderPlayer(game_offscreen_buffer* Buffer, game_asset* Asset,
    bool Highlight)
{
    u8* Out = (u8*)Buffer->Memory;/*AARRGGBB*/
    u8*  In = (u8*) Asset->Pixels;  /*BBGGRR*/
    //NOTE:Looks like not a nice way but fine for now
    if(4 < Asset->X && 4 < Asset->Y &&
       Asset->X < 1037 && Asset->Y < 606)
    {
        Out += 4*Asset->X;
        Out += Asset->Y*Buffer->Pitch;
        for(s32 Y = 0; Y < Asset->Height; Y++)
        {
            u32* out = (u32*)Out;
            u8*  in = (u8*)In;
            for(s32 X = 0; X < Asset->Width; X++)
            {
                *out++ = *(in) | *(in + 1)<<8 | *(in + 2)<<16;
                in+=3;
            }
            Out += Buffer->Pitch;
            In += Asset->Pitch;
        }
        if(Highlight)
        {
            double_point Rectangle;
            Rectangle.X1 = Asset->X + Asset->Width/2  - 23;
            Rectangle.Y1 = Asset->Y + Asset->Height/2 - 23;
            Rectangle.X2 = Asset->X + Asset->Width/2  + 22;
            Rectangle.Y2 = Asset->Y + Asset->Height/2 + 22;
            DrawRectangle(Buffer, Rectangle.X1, Rectangle.Y1,
                          Rectangle.X2, Rectangle.Y2, 0xffffffff);
        }
    }
    
}

internal void
PlayersRendering(game_offscreen_buffer* Buffer, extra_info* Extra,
                 position* Positions, player_info* Players,
                 game_asset* PlayerAsset1, game_asset* PlayerAsset2)
{
    for(u32 Index = 0; Index < 24; Index++)
    {
        if((Players[Index].Player == Player1))
        {
            PlayerAsset1->X = Players[Index].X - 20;
            PlayerAsset1->Y = Players[Index].Y - 20;
            RenderPlayer(Buffer, PlayerAsset1, Players[Index].Highlight);
        }
        else
        {
            PlayerAsset2->X = Players[Index].X - 20;
            PlayerAsset2->Y = Players[Index].Y - 20;
            RenderPlayer(Buffer, PlayerAsset2, Players[Index].Highlight);
        }
    }
}

internal void
IsWin()
{

}

internal void
GameUpdateAndRender(game_offscreen_buffer *Buffer, extra_info* Extra,
                    position* Positions, player_info* Players,
                    game_asset* PlayerAsset1, game_asset* PlayerAsset2)
{
    GameBackground(Buffer);
    GamePattern(Buffer, Extra);
    PlayersRendering(Buffer, Extra, Positions, 
                     Players, PlayerAsset1, PlayerAsset2);
    IsWin();
}

internal void
PlayersInitialization(player_info* Players, extra_info* Extra,
                      game_asset* PlayerAsset1, game_asset* PlayerAsset2)
{
    for(u32 Index = 0; Index < 12; Index++)
    {
        Players[Index].Player    = Player2;
        Players[Index].Index     = Index;
        Players[Index].X         = Extra->MarginWidth  + Extra->SquareLength*(Index%5);
        Players[Index].Y         = Extra->MarginHeight + Extra->SquareLength*(Index/5);
        Players[Index].Highlight = false;
        Players[Index].IsAlive   = Alive;
        Players[Index].Asset     = PlayerAsset2;
    }
    for(u32 Index = 12; Index < 24; Index++)
    {
        Players[Index].Player    = Player1;
        Players[Index].Index     = Index;
        Players[Index].X         = Extra->MarginWidth  + Extra->SquareLength*((Index + 1)%5);
        Players[Index].Y         = Extra->MarginHeight + Extra->SquareLength*((Index + 1)/5);
        Players[Index].Highlight = false;
        Players[Index].IsAlive   = Alive;
        Players[Index].Asset     = PlayerAsset1;
    }
}

internal void
InitializePositions(position* Positions, extra_info* Extra, player_info* Players)
{
    for(s32 Index = 0; Index < 12; Index++)
    {
        Positions[Index].Index = Index;
        Positions[Index].X = Extra->MarginWidth
            + Extra->SquareLength*(Index%5);
        Positions[Index].Y = Extra->MarginHeight
            + Extra->SquareLength*(Index/5);
        Positions[Index].Player = &Players[Index];
    }
    Positions[12].Index  = 12;
    Positions[12].X      = Extra->MarginWidth  + Extra->SquareLength*(12%5);
    Positions[12].Y      = Extra->MarginHeight + Extra->SquareLength*(12/5);
    Positions[12].Player = NULL;
    for(s32 Index = 13; Index < 25; Index++)
    {
        Positions[Index].Index = Index;
        Positions[Index].X = Extra->MarginWidth
            + Extra->SquareLength*(Index%5);
        Positions[Index].Y = Extra->MarginHeight
            + Extra->SquareLength*(Index/5);
        Positions[Index].Player = &Players[Index - 1];
    }
}

internal void
HighlightTile(point* MousePointer, position* Positions, 
              extra_info* Extra, whos_turn* Turn)
{
    for(u32 Index = 0; Index < 25; Index++)
    {
        if(Positions[Index].Player != NULL)
        {
            if((*Turn == MyTurn &&
                Positions[Index].Player->Player == Player1) ||
               (*Turn == YourTurn &&
                Positions[Index].Player->Player == Player2))
            {
                point IndexCoordinates;
                IndexCoordinates.X = Positions[Index].Player->X;
                IndexCoordinates.Y = Positions[Index].Player->Y;
     
                if(IndexCoordinates.X - 20 < MousePointer->X &&
                   IndexCoordinates.X + 20 > MousePointer->X &&
                   IndexCoordinates.Y - 20 < MousePointer->Y &&
                   IndexCoordinates.Y + 20 > MousePointer->Y)
                {
                    Positions[Index].Player->Highlight = true;
                }
                else
                    Positions[Index].Player->Highlight = false;
            }
        }
    }
}

internal bool
IsPointerOnTile(point* MousePointer, position* Positions, 
                extra_info* Extra, whos_turn* Turn, u32* Index)
{
    for(*Index = 0; *Index < 25; (*Index)++)
    {
        if(Positions[*Index].Player != NULL)
        {
            if((*Turn == MyTurn &&
                Positions[*Index].Player->Player == Player1) ||
               (*Turn == YourTurn &&
                Positions[*Index].Player->Player == Player2))
            {
                point IndexCoordinates;
                IndexCoordinates.X = Positions[*Index].Player->X;
                IndexCoordinates.Y = Positions[*Index].Player->Y;
                if(IndexCoordinates.X - 20 < MousePointer->X &&
                   IndexCoordinates.X + 20 > MousePointer->X &&
                   IndexCoordinates.Y - 20 < MousePointer->Y &&
                   IndexCoordinates.Y + 20 > MousePointer->Y)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

void internal
SlidePlayer(point* MousePointer, position* Positions, u32* Index, point* DeltaPointer)
{
    DeltaPointer->X = MousePointer->X - Positions[*Index].X;
    DeltaPointer->Y = MousePointer->Y - Positions[*Index].Y;
    float DyByDx = (float)Mod(DeltaPointer->Y)/(float)Mod(DeltaPointer->X);
    if(DyByDx > 1.732f)
    {
        Positions[*Index].Player->Y = Positions[*Index].Y + DeltaPointer->Y;
        Positions[*Index].Player->X = Positions[*Index].X;

    }
    else if(DyByDx < 0.577f)
    {
        Positions[*Index].Player->X = Positions[*Index].X + DeltaPointer->X;
        Positions[*Index].Player->Y = Positions[*Index].Y;
    }
    else
    {
        if(*Index%2 == 0)
        {
            if(DeltaPointer->X < 0 && DeltaPointer->Y < 0)
            {
                Positions[*Index].Player->X = Positions[*Index].X + DeltaPointer->X;
                Positions[*Index].Player->Y = Positions[*Index].Y + DeltaPointer->X;
            }
            else if(DeltaPointer->X < 0 && DeltaPointer->Y > 0)
            {
                Positions[*Index].Player->X = Positions[*Index].X + DeltaPointer->X;
                Positions[*Index].Player->Y = Positions[*Index].Y - DeltaPointer->X;
            }
            else if(DeltaPointer->X > 0 && DeltaPointer->Y < 0)
            {
                Positions[*Index].Player->X = Positions[*Index].X + DeltaPointer->X;
                Positions[*Index].Player->Y = Positions[*Index].Y - DeltaPointer->X;
            }
            else
            {
                Positions[*Index].Player->X = Positions[*Index].X + DeltaPointer->X;
                Positions[*Index].Player->Y = Positions[*Index].Y + DeltaPointer->X;
            }
        }
        else
        {
            if(DyByDx > 1.0f)
            {
                Positions[*Index].Player->Y = Positions[*Index].Y + DeltaPointer->Y;
                Positions[*Index].Player->X = Positions[*Index].X;

            }
            else
            {
                Positions[*Index].Player->X = Positions[*Index].X + DeltaPointer->X;
                Positions[*Index].Player->Y = Positions[*Index].Y;
            }
        }
    }
}

internal bool
ShouldPlayerMove(position* Positions, u32* Index, point* DeltaPointer, extra_info* Extra, u32* DestinationIndex, bool* ShouldKill, u32* DestroyedIndex)
{
    float DyByDx = (float)Mod(DeltaPointer->Y)/(float)Mod(DeltaPointer->X);

    if(DyByDx > 1.732f)
    {
        if(Mod(DeltaPointer->Y) > Extra->SquareLength/2)
        {
            if(DeltaPointer->Y > 0)
            {
                if(*Index/5 < 4)
                {
                    if(Positions[*Index + 5].Player == NULL)
                    {
                        *DestinationIndex = *Index + 5;
                        return true;
                    }
                    else if(*Index/5 < 3)
                    {
                        if(Positions[*Index + 10].Player == NULL)
                        {
                            if(Positions[*Index + 5].Player->Player != Positions[*Index].Player->Player)
                            {
                                *ShouldKill = true;
                                *DestroyedIndex = *Index + 5;
                                *DestinationIndex = *Index + 10;
                                return true;
                            }
                        }
                    }
                }
            }
            if(DeltaPointer->Y < 0)
            {
                if(0 < *Index/5)
                {
                    if(Positions[*Index - 5].Player == NULL)
                    {
                        *DestinationIndex = *Index - 5;
                        return true;
                    }
                    else if(1 < *Index/5)
                    {
                        if(Positions[*Index - 10].Player == NULL)
                        {
                            if(Positions[*Index - 5].Player->Player != Positions[*Index].Player->Player)
                            {
                                *ShouldKill = true;
                                *DestroyedIndex = *Index - 5;
                                *DestinationIndex = *Index - 10;
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    else if(DyByDx < 0.577f)
    {
        if(Mod(DeltaPointer->X) > Extra->SquareLength/2)
        {
            if(DeltaPointer->X > 0)
            {
                if(*Index%5 < 4)
                {
                    if(Positions[*Index + 1].Player == NULL)
                    {
                        *DestinationIndex = *Index + 1;
                        return true;
                    }
                    else if(*Index%5 < 3)
                    {
                        if(Positions[*Index + 2].Player == NULL)
                        {
                            if(Positions[*Index + 1].Player->Player != Positions[*Index].Player->Player)
                            {
                                *ShouldKill = true;
                                *DestroyedIndex = *Index + 1;
                                *DestinationIndex = *Index + 2;
                                return true;
                            }
                        }
                    }
                }
            }
            if(DeltaPointer->X < 0)
            {
                if(0 < *Index%5)
                {
                    if(Positions[*Index - 1].Player == NULL)
                    {
                        *DestinationIndex = *Index - 1;
                        return true;
                    }
                    else if(1 < *Index%5)
                    {
                        if(Positions[*Index - 2].Player == NULL)
                        {
                            if(Positions[*Index - 1].Player->Player != Positions[*Index].Player->Player)
                            {
                                *ShouldKill = true;
                                *DestroyedIndex = *Index - 1;
                                *DestinationIndex = *Index - 2;
                                return true;
                            }
                        }
                    }
                }
            }
        }
    }
    else
    {
        if(*Index%2 == 0)
        {
            if(DeltaPointer->X < 0 && DeltaPointer->Y < 0)
            {
                if(Mod(DeltaPointer->X) > Extra->SquareLength/2)
                {
                    if(0 < *Index%5 && 0 < *Index/5)
                    {
                        if(Positions[*Index - 1 - 5].Player == NULL)
                        {
                            *DestinationIndex = *Index - 1- 5;
                            return true;
                        }
                        else if(1 < *Index%5 && 1 < *Index/5)
                        {
                            if(Positions[*Index - 1 - 5].Player->Player != Positions[*Index].Player->Player)
                            {
                                *ShouldKill = true;
                                *DestroyedIndex = *Index - 1 - 5;
                                *DestinationIndex = *Index - 2 - 10;
                                return true;
                            }
                        }
                    }
                }
            }
            else if(DeltaPointer->X < 0 && DeltaPointer->Y > 0)
            {
                if(Mod(DeltaPointer->X) > Extra->SquareLength/5)
                {
                    if(0 < *Index%5 && *Index/5 < 4)
                    {
                        if(Positions[*Index - 1 + 5].Player == NULL)
                        {
                            *DestinationIndex = *Index - 1 + 5;
                            return true;
                        }
                        else if(1 < *Index%5 && *Index/5 < 3)
                        {
                            if(Positions[*Index - 1 + 5].Player->Player != Positions[*Index].Player->Player)
                            {
                                *ShouldKill = true;
                                *DestroyedIndex = *Index - 1 + 5;
                                *DestinationIndex = *Index - 2 + 10;
                                return true;
                            }
                        }
                    }
                }
            }
            else if(DeltaPointer->X > 0 && DeltaPointer->Y < 0)
            {
                if(Mod(DeltaPointer->X) > Extra->SquareLength/2)
                {
                    if(*Index%5 < 4 && 0 < *Index/5)
                    {
                        if(Positions[*Index + 1 - 5].Player == NULL)
                        {
                            *DestinationIndex = *Index + 1 - 5;
                            return true;
                        }
                        else if(*Index%5 < 3 && 1 < *Index/5)
                        {
                            if(Positions[*Index + 1 - 5].Player->Player != Positions[*Index].Player->Player)
                            {
                                *ShouldKill = true;
                                *DestroyedIndex = *Index + 1 - 5;
                                *DestinationIndex = *Index + 2 - 10;
                                return true;
                            }
                        }
                    }
                }
            }
            else
            {
                if(Mod(DeltaPointer->X) > Extra->SquareLength/2)
                {
                    if(*Index%5 < 4 && *Index/5 < 4)
                    {
                        if(Positions[*Index + 1 + 5].Player == NULL)
                        {
                            *DestinationIndex = *Index + 1 + 5;
                            return true;
                        }
                        else if(*Index%5 < 3 && *Index/5 < 3)
                        {
                            if(Positions[*Index + 1 + 5].Player->Player != Positions[*Index].Player->Player)
                            {
                                *ShouldKill = true;
                                *DestroyedIndex = *Index + 1 + 5;
                                *DestinationIndex = *Index + 2 + 10;
                                return true;
                            }
                        }
                    }
                }
            }
        }
        else
        {
            if(DyByDx > 1.0f)
            {
                if(Mod(DeltaPointer->X) > Extra->SquareLength/2)
                {
                    
                }
            }
            else
            {
                
            }
        }
    }
    return false;
}

internal void
MovePlayer(position* Positions, u32* Index, u32* DestinationIndex)
{
    Positions[*DestinationIndex].Player = Positions[*Index].Player;
    Positions[*DestinationIndex].Player->X = Positions[*DestinationIndex].X;
    Positions[*DestinationIndex].Player->Y = Positions[*DestinationIndex].Y;
    Positions[*Index].Player = NULL;
}

internal void
ToggleTurn(whos_turn* Turn)
{
    if(*Turn == MyTurn)
    {
        *Turn = YourTurn;
    }
    else
    {
        *Turn = MyTurn;
    }
}

internal void
KillPlayer(position* Positions, player_info* Players, u32* Index, u32* DestinationIndex, u32* DestroyedIndex)
{
    Positions[*DestinationIndex].Player = Positions[*Index].Player;
    Positions[*DestinationIndex].Player->X = Positions[*DestinationIndex].X;
    Positions[*DestinationIndex].Player->Y = Positions[*DestinationIndex].Y;
    
    player_info *Player = Positions[*DestroyedIndex].Player;
    if(Player->Player == Player1)
    {
        u32 DeadPlayersCount = 0;
        for(u32 Counter = 12; Counter < 24; Counter++)
        {
            if(Players[Counter].IsAlive == Dead)
            {
                DeadPlayersCount++;
            }
        }
        Player->X = 160;
        Player->Y = 60 + 50*DeadPlayersCount;
        Player->Highlight = false;
        Player->IsAlive = Dead;
    }
    else
    {
        u32 DeadPlayersCount = 0;
        for(u32 Counter = 0; Counter < 12; Counter++)
        {
            if(Players[Counter].IsAlive == Dead)
            {
                DeadPlayersCount++;
            }
        }
        Player->X = 920;
        Player->Y = 60 + 50*DeadPlayersCount;
        Player->Highlight = false;
        Player->IsAlive = Dead;    
    }

    Positions[*DestroyedIndex].Player = NULL;
    Positions[*Index].Player = NULL;
}
