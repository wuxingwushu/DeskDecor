/******************************************************************************
 * | File      	:   GUI_Paint.c
 * | Author      :   Waveshare electronics
 * | Function    :	Achieve drawing: draw points, lines, boxes, circles and
 *                   their size, solid dotted line, solid rectangle hollow
 *                   rectangle, solid circle hollow circle.
 * | Info        :
 *   Achieve display characters: Display a single character, string, number
 *   Achieve time display: adaptive size display time minutes and seconds
 *----------------
 * |	This version:   V3.2
 * | Date        :   2020-07-23
 * | Info        :
 * -----------------------------------------------------------------------------
 * V3.2(2020-07-23):
 * 1. Change: Paint_SetScale(UBYTE scale)
 *			Add scale 7 for 5.65f e-Parper
 * 2. Change: Paint_SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color)
 * 			Add the branch for scale 7
 * 3. Change: Paint_Clear(UWORD Color)
 *			Add the branch for scale 7
 *
 * V3.1(2019-10-10):
 * 1. Add gray level
 *   PAINT Add Scale
 * 2. Add void Paint_SetScale(UBYTE scale);
 *
 * V3.0(2019-04-18):
 * 1.Change:
 *    Paint_DrawPoint(..., DOT_STYLE DOT_STYLE)
 * => Paint_DrawPoint(..., DOT_STYLE Dot_Style)
 *    Paint_DrawLine(..., LINE_STYLE Line_Style, DOT_PIXEL Dot_Pixel)
 * => Paint_DrawLine(..., DOT_PIXEL Line_width, LINE_STYLE Line_Style)
 *    Paint_DrawRectangle(..., DRAW_FILL Filled, DOT_PIXEL Dot_Pixel)
 * => Paint_DrawRectangle(..., DOT_PIXEL Line_width, DRAW_FILL Draw_Fill)
 *    Paint_DrawCircle(..., DRAW_FILL Draw_Fill, DOT_PIXEL Dot_Pixel)
 * => Paint_DrawCircle(..., DOT_PIXEL Line_width, DRAW_FILL Draw_Filll)
 *
 * -----------------------------------------------------------------------------
 * V2.0(2018-11-15):
 * 1.add: Paint_NewImage()
 *    Create an image's properties
 * 2.add: Paint_SelectImage()
 *    Select the picture to be drawn
 * 3.add: Paint_SetRotate()
 *    Set the direction of the cache
 * 4.add: Paint_RotateImage()
 *    Can flip the picture, Support 0-360 degrees,
 *    but only 90.180.270 rotation is better
 * 4.add: Paint_SetMirroring()
 *    Can Mirroring the picture, horizontal, vertical, origin
 * 5.add: Paint_DrawString_CN()
 *    Can display Chinese(GB1312)
 *
 * -----------------------------------------------------------------------------
 * V1.0(2018-07-17):
 *   Create library
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documnetation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to  whom the Software is
 * furished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS OR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 ******************************************************************************/
#include "GUI_Paint.h"
#include "DEV_Config.h"
#include "Debug.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h> //memset()
#include <math.h>
#include "FontFunction.h"
PAINT Paint;

/******************************************************************************
function: Create Image
parameter:
    image   :   Pointer to the image cache
    width   :   The width of the picture
    Height  :   The height of the picture
    Color   :   Whether the picture is inverted
******************************************************************************/
void Paint_NewImage(UBYTE *image, UWORD Width, UWORD Height, UWORD Rotate, UWORD Color)
{
    Paint.Image = NULL;
    Paint.Image = image;

    Paint.WidthMemory = Width;
    Paint.HeightMemory = Height;
    Paint.Color = Color;
    Paint.Scale = 2;
    Paint.WidthByte = (Width % 8 == 0) ? (Width / 8) : (Width / 8 + 1);
    Paint.HeightByte = Height;
    //    printf("WidthByte = %d, HeightByte = %d\r\n", Paint.WidthByte, Paint.HeightByte);
    //    printf(" EPD_WIDTH / 8 = %d\r\n",  122 / 8);

    Paint.Rotate = Rotate;
    Paint.Mirror = MIRROR_NONE;

    if (Rotate == ROTATE_0 || Rotate == ROTATE_180)
    {
        Paint.Width = Width;
        Paint.Height = Height;
    }
    else
    {
        Paint.Width = Height;
        Paint.Height = Width;
    }
}

/******************************************************************************
function: Select Image
parameter:
    image : Pointer to the image cache
******************************************************************************/
void Paint_SelectImage(UBYTE *image)
{
    Paint.Image = image;
}

/******************************************************************************
function: Select Image Rotate
parameter:
    Rotate : 0,90,180,270
******************************************************************************/
void Paint_SetRotate(UWORD Rotate)
{
    if (Rotate == ROTATE_0 || Rotate == ROTATE_90 || Rotate == ROTATE_180 || Rotate == ROTATE_270)
    {
        // Debug("Set image Rotate %d\r\n", Rotate);
        Paint.Rotate = Rotate;
    }
    else
    {
        Debug("rotate = 0, 90, 180, 270\r\n");
    }
}

/******************************************************************************
function:	Select Image mirror
parameter:
    mirror   :Not mirror,Horizontal mirror,Vertical mirror,Origin mirror
******************************************************************************/
void Paint_SetMirroring(UBYTE mirror)
{
    if (mirror == MIRROR_NONE || mirror == MIRROR_HORIZONTAL ||
        mirror == MIRROR_VERTICAL || mirror == MIRROR_ORIGIN)
    {
        // Debug("mirror image x:%s, y:%s\r\n",(mirror & 0x01)? "mirror":"none", ((mirror >> 1) & 0x01)? "mirror":"none");
        Paint.Mirror = mirror;
    }
    else
    {
        Debug("mirror should be MIRROR_NONE, MIRROR_HORIZONTAL, \
        MIRROR_VERTICAL or MIRROR_ORIGIN\r\n");
    }
}

void Paint_SetScale(UBYTE scale)
{
    if (scale == 2)
    {
        Paint.Scale = scale;
        Paint.WidthByte = (Paint.WidthMemory % 8 == 0) ? (Paint.WidthMemory / 8) : (Paint.WidthMemory / 8 + 1);
    }
    else if (scale == 4)
    {
        Paint.Scale = scale;
        Paint.WidthByte = (Paint.WidthMemory % 4 == 0) ? (Paint.WidthMemory / 4) : (Paint.WidthMemory / 4 + 1);
    }
    else if (scale == 7)
    { // Only applicable with 5in65 e-Paper
        Paint.Scale = 7;
        Paint.WidthByte = (Paint.WidthMemory % 2 == 0) ? (Paint.WidthMemory / 2) : (Paint.WidthMemory / 2 + 1);
    }
    else
    {
        Debug("Set Scale Input parameter error\r\n");
        Debug("Scale Only support: 2 4 7\r\n");
    }
}
/******************************************************************************
function: Draw Pixels
parameter:
    Xpoint : At point X
    Ypoint : At point Y
    Color  : Painted colors
******************************************************************************/
void Paint_SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color)
{
    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Exceeding display boundaries\r\n");
        return;
    }
    UWORD X, Y;
    switch (Paint.Rotate)
    {
    case 0:
        X = Xpoint;
        Y = Ypoint;
        break;
    case 90:
        X = Paint.WidthMemory - Ypoint - 1;
        Y = Xpoint;
        break;
    case 180:
        X = Paint.WidthMemory - Xpoint - 1;
        Y = Paint.HeightMemory - Ypoint - 1;
        break;
    case 270:
        X = Ypoint;
        Y = Paint.HeightMemory - Xpoint - 1;
        break;
    default:
        return;
    }

    switch (Paint.Mirror)
    {
    case MIRROR_NONE:
        break;
    case MIRROR_HORIZONTAL:
        X = Paint.WidthMemory - X - 1;
        break;
    case MIRROR_VERTICAL:
        Y = Paint.HeightMemory - Y - 1;
        break;
    case MIRROR_ORIGIN:
        X = Paint.WidthMemory - X - 1;
        Y = Paint.HeightMemory - Y - 1;
        break;
    default:
        return;
    }

    if (X > Paint.WidthMemory || Y > Paint.HeightMemory)
    {
        Debug("Exceeding display boundaries\r\n");
        return;
    }

    if (Paint.Scale == 2)
    {
        UDOUBLE Addr = X / 8 + Y * Paint.WidthByte;
        UBYTE Rdata = Paint.Image[Addr];
        if (Color == BLACK)
            Paint.Image[Addr] = Rdata & ~(0x80 >> (X % 8));
        else
            Paint.Image[Addr] = Rdata | (0x80 >> (X % 8));
    }
    else if (Paint.Scale == 4)
    {
        UDOUBLE Addr = X / 4 + Y * Paint.WidthByte;
        Color = Color % 4; // Guaranteed color scale is 4  --- 0~3
        UBYTE Rdata = Paint.Image[Addr];

        Rdata = Rdata & (~(0xC0 >> ((X % 4) * 2)));
        Paint.Image[Addr] = Rdata | ((Color << 6) >> ((X % 4) * 2));
    }
    else if (Paint.Scale == 7)
    {
        UWORD Width = Paint.WidthMemory * 3 % 8 == 0 ? Paint.WidthMemory * 3 / 8 : Paint.WidthMemory * 3 / 8 + 1;
        UDOUBLE Addr = (Xpoint * 3) / 8 + Ypoint * Width;
        UBYTE shift, Rdata, Rdata2;
        shift = (Xpoint + Ypoint * Paint.HeightMemory) % 8;

        switch (shift)
        {
        case 0:
            Rdata = Paint.Image[Addr] & 0x1f;
            Rdata = Rdata | ((Color << 5) & 0xe0);
            Paint.Image[Addr] = Rdata;
            break;
        case 1:
            Rdata = Paint.Image[Addr] & 0xe3;
            Rdata = Rdata | ((Color << 2) & 0x1c);
            Paint.Image[Addr] = Rdata;
            break;
        case 2:
            Rdata = Paint.Image[Addr] & 0xfc;
            Rdata2 = Paint.Image[Addr + 1] & 0x7f;
            Rdata = Rdata | ((Color >> 1) & 0x03);
            Rdata2 = Rdata2 | ((Color << 7) & 0x80);
            Paint.Image[Addr] = Rdata;
            Paint.Image[Addr + 1] = Rdata2;
            break;
        case 3:
            Rdata = Paint.Image[Addr] & 0x8f;
            Rdata = Rdata | ((Color << 4) & 0x70);
            Paint.Image[Addr] = Rdata;
            break;
        case 4:
            Rdata = Paint.Image[Addr] & 0xf1;
            Rdata = Rdata | ((Color << 1) & 0x0e);
            Paint.Image[Addr] = Rdata;
            break;
        case 5:
            Rdata = Paint.Image[Addr] & 0xfe;
            Rdata2 = Paint.Image[Addr + 1] & 0x3f;
            Rdata = Rdata | ((Color >> 2) & 0x01);
            Rdata2 = Rdata2 | ((Color << 6) & 0xc0);
            Paint.Image[Addr] = Rdata;
            Paint.Image[Addr + 1] = Rdata2;
            break;
        case 6:
            Rdata = Paint.Image[Addr] & 0xc7;
            Rdata = Rdata | ((Color << 3) & 0x38);
            Paint.Image[Addr] = Rdata;
            break;
        case 7:
            Rdata = Paint.Image[Addr] & 0xf8;
            Rdata = Rdata | (Color & 0x07);
            Paint.Image[Addr] = Rdata;
            break;
        }
    }
}

void Paint_SetPixel_Gai(UWORD Xpoint, UWORD Ypoint, UWORD Color)
{
    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Exceeding display boundaries\r\n");
        return;
    }
    UWORD X, Y;
    X = Paint.WidthMemory - Ypoint - 1;
    Y = Xpoint;
    UDOUBLE Addr = X / 8 + Y * Paint.WidthByte;
    X = (0x80 >> (X % 8));
    UBYTE Rdata = Paint.Image[Addr];
    if (Color == BLACK)
        Paint.Image[Addr] = Rdata & ~X;
    else
        Paint.Image[Addr] = Rdata | X;
}

/******************************************************************************
function: Clear the color of the picture
parameter:
    Color : Painted colors
******************************************************************************/
void Paint_Clear(UWORD Color)
{
    if (Paint.Scale == 2 || Paint.Scale == 4)
    {
        for (UWORD Y = 0; Y < Paint.HeightByte; Y++)
        {
            for (UWORD X = 0; X < Paint.WidthByte; X++)
            { // 8 pixel =  1 byte
                UDOUBLE Addr = X + Y * Paint.WidthByte;
                Paint.Image[Addr] = Color;
            }
        }
    }
    if (Paint.Scale == 7)
    {
        Color = (UBYTE)Color;
        UWORD Width = (Paint.WidthMemory * 3 % 8 == 0) ? (Paint.WidthMemory * 3 / 8) : (Paint.WidthMemory * 3 / 8 + 1);
        for (UWORD Y = 0; Y < Paint.HeightByte; Y++)
        {
            for (UWORD X = 0; X < Width; X++)
            {
                UDOUBLE Addr = X + Y * Width;
                if ((X + Y * Width) % 3 == 0)
                    Paint.Image[Addr] = ((Color << 5) | (Color << 2) | (Color >> 1));
                else if ((X + Y * Width) % 3 == 1)
                    Paint.Image[Addr] = ((Color << 7) | (Color << 4) | (Color << 1) | (Color >> 2));
                else if ((X + Y * Width) % 3 == 2)
                    Paint.Image[Addr] = ((Color << 6) | (Color << 3) | Color);
            }
        }
    }
}

/******************************************************************************
function: Clear the color of a window
parameter:
    Xstart : x starting point
    Ystart : Y starting point
    Xend   : x end point
    Yend   : y end point
    Color  : Painted colors
******************************************************************************/
void Paint_ClearWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color)
{
    UWORD X, Y;
    for (Y = Ystart; Y < Yend; Y++)
    {
        for (X = Xstart; X < Xend; X++)
        { // 8 pixel =  1 byte
            Paint_SetPixel(X, Y, Color);
        }
    }
}

/******************************************************************************
function: Draw Point(Xpoint, Ypoint) Fill the color
parameter:
    Xpoint		: The Xpoint coordinate of the point
    Ypoint		: The Ypoint coordinate of the point
    Color		: Painted color
    Dot_Pixel	: point size
    Dot_Style	: point Style
******************************************************************************/
void Paint_DrawPoint(UWORD Xpoint, UWORD Ypoint, UWORD Color,
                     DOT_PIXEL Dot_Pixel, DOT_STYLE Dot_Style)
{
    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Paint_DrawPoint Input exceeds the normal display range\r\n");
        return;
    }

    int16_t XDir_Num, YDir_Num;
    if (Dot_Style == DOT_FILL_AROUND)
    {
        for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++)
        {
            for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++)
            {
                if (Xpoint + XDir_Num - Dot_Pixel < 0 || Ypoint + YDir_Num - Dot_Pixel < 0)
                    break;
                // printf("x = %d, y = %d\r\n", Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel);
                Paint_SetPixel(Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel, Color);
            }
        }
    }
    else
    {
        for (XDir_Num = 0; XDir_Num < Dot_Pixel; XDir_Num++)
        {
            for (YDir_Num = 0; YDir_Num < Dot_Pixel; YDir_Num++)
            {
                Paint_SetPixel(Xpoint + XDir_Num - 1, Ypoint + YDir_Num - 1, Color);
            }
        }
    }
}

/******************************************************************************
function: Draw a line of arbitrary slope
parameter:
    Xstart ：Starting Xpoint point coordinates
    Ystart ：Starting Xpoint point coordinates
    Xend   ：End point Xpoint coordinate
    Yend   ：End point Ypoint coordinate
    Color  ：The color of the line segment
    Line_width : Line width
    Line_Style: Solid and dotted lines
******************************************************************************/
void Paint_DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,
                    UWORD Color, DOT_PIXEL Line_width, LINE_STYLE Line_Style)
{
    if (Xstart > Paint.Width || Ystart > Paint.Height ||
        Xend > Paint.Width || Yend > Paint.Height)
    {
        Debug("Paint_DrawLine Input exceeds the normal display range\r\n");
        return;
    }

    UWORD Xpoint = Xstart;
    UWORD Ypoint = Ystart;
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

    // Increment direction, 1 is positive, -1 is counter;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;

    // Cumulative error
    int Esp = dx + dy;
    char Dotted_Len = 0;

    for (;;)
    {
        Dotted_Len++;
        // Painted dotted line, 2 point is really virtual
        if (Line_Style == LINE_STYLE_DOTTED && Dotted_Len % 3 == 0)
        {
            // Debug("LINE_DOTTED\r\n");
            Paint_DrawPoint(Xpoint, Ypoint, IMAGE_BACKGROUND, Line_width, DOT_STYLE_DFT);
            Dotted_Len = 0;
        }
        else
        {
            Paint_DrawPoint(Xpoint, Ypoint, Color, Line_width, DOT_STYLE_DFT);
        }
        if (2 * Esp >= dy)
        {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx)
        {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

/******************************************************************************
function: Draw a rectangle
parameter:
    Xstart ：Rectangular  Starting Xpoint point coordinates
    Ystart ：Rectangular  Starting Xpoint point coordinates
    Xend   ：Rectangular  End point Xpoint coordinate
    Yend   ：Rectangular  End point Ypoint coordinate
    Color  ：The color of the Rectangular segment
    Line_width: Line width
    Draw_Fill : Whether to fill the inside of the rectangle
******************************************************************************/
void Paint_DrawRectangle(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,
                         UWORD Color, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill)
{
    if (Xstart > Paint.Width || Ystart > Paint.Height ||
        Xend > Paint.Width || Yend > Paint.Height)
    {
        Debug("Input exceeds the normal display range\r\n");
        return;
    }

    if (Draw_Fill)
    {
        UWORD Ypoint;
        for (Ypoint = Ystart; Ypoint < Yend; Ypoint++)
        {
            Paint_DrawLine(Xstart, Ypoint, Xend, Ypoint, Color, Line_width, LINE_STYLE_SOLID);
        }
    }
    else
    {
        Paint_DrawLine(Xstart, Ystart, Xend, Ystart, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xstart, Ystart, Xstart, Yend, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xend, Yend, Xend, Ystart, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xend, Yend, Xstart, Yend, Color, Line_width, LINE_STYLE_SOLID);
    }
}

/******************************************************************************
function: Use the 8-point method to draw a circle of the
            specified size at the specified position->
parameter:
    X_Center  ：Center X coordinate
    Y_Center  ：Center Y coordinate
    Radius    ：circle Radius
    Color     ：The color of the ：circle segment
    Line_width: Line width
    Draw_Fill : Whether to fill the inside of the Circle
******************************************************************************/
void Paint_DrawCircle(UWORD X_Center, UWORD Y_Center, UWORD Radius,
                      UWORD Color, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill)
{
    if (X_Center > Paint.Width || Y_Center >= Paint.Height)
    {
        Debug("Paint_DrawCircle Input exceeds the normal display range\r\n");
        return;
    }

    // Draw a circle from(0, R) as a starting point
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;

    // Cumulative error,judge the next point of the logo
    int16_t Esp = 3 - (Radius << 1);

    int16_t sCountY;
    if (Draw_Fill == DRAW_FILL_FULL)
    {
        while (XCurrent <= YCurrent)
        { // Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY++)
            {
                Paint_DrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 1
                Paint_DrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 2
                Paint_DrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 3
                Paint_DrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 4
                Paint_DrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 5
                Paint_DrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 6
                Paint_DrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT); // 7
                Paint_DrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if (Esp < 0)
                Esp += 4 * XCurrent + 6;
            else
            {
                Esp += 10 + 4 * (XCurrent - YCurrent);
                YCurrent--;
            }
            XCurrent++;
        }
    }
    else
    { // Draw a hollow circle
        while (XCurrent <= YCurrent)
        {
            Paint_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color, Line_width, DOT_STYLE_DFT); // 1
            Paint_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color, Line_width, DOT_STYLE_DFT); // 2
            Paint_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color, Line_width, DOT_STYLE_DFT); // 3
            Paint_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color, Line_width, DOT_STYLE_DFT); // 4
            Paint_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color, Line_width, DOT_STYLE_DFT); // 5
            Paint_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color, Line_width, DOT_STYLE_DFT); // 6
            Paint_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color, Line_width, DOT_STYLE_DFT); // 7
            Paint_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color, Line_width, DOT_STYLE_DFT); // 0

            if (Esp < 0)
                Esp += 4 * XCurrent + 6;
            else
            {
                Esp += 10 + 4 * (XCurrent - YCurrent);
                YCurrent--;
            }
            XCurrent++;
        }
    }
}

/******************************************************************************
function:	Display monochrome bitmap
parameter:
    image_buffer ：A picture data converted to a bitmap
info:
    Use a computer to convert the image into a corresponding array,
    and then embed the array directly into Imagedata.cpp as a .c file.
******************************************************************************/
void Paint_DrawBitMap(const unsigned char *image_buffer)
{
    UWORD x, y;
    UDOUBLE Addr = 0;

    for (y = 0; y < Paint.HeightByte; y++)
    {
        for (x = 0; x < Paint.WidthByte; x++)
        { // 8 pixel =  1 byte
            Addr = x + y * Paint.WidthByte;
            Paint.Image[Addr] = (unsigned char)image_buffer[Addr];
        }
    }
}

/******************************************************************************
function:	Display image
parameter:
    image            ：Image start address
    xStart           : X starting coordinates
    yStart           : Y starting coordinates
    xEnd             ：Image width
    yEnd             : Image height
******************************************************************************/
void Paint_DrawImage(const unsigned char *image_buffer, UWORD xStart, UWORD yStart, UWORD W_Image, UWORD H_Image)
{
    UWORD x, y;
    UWORD w_byte = (W_Image % 8) ? (W_Image / 8) + 1 : W_Image / 8;
    UDOUBLE Addr = 0;
    UDOUBLE pAddr = 0;
    for (y = 0; y < H_Image; y++)
    {
        for (x = 0; x < w_byte; x++)
        { // 8 pixel =  1 byte
            Addr = x + y * w_byte;
            pAddr = x + (xStart / 8) + ((y + yStart) * Paint.WidthByte);
            Paint.Image[pAddr] = (unsigned char)image_buffer[Addr];
        }
    }
}

File WordInfoFile; // 字体信息文件
File WordImgFile;  // 字体位图文件
unsigned int CN_UTF8_Show(UWORD Xstart, UWORD Ystart, unsigned short filename)
{
    // 字体不存在
    if (filename == 0xFFFF)
    {
        return 0;
    }
    UWORD x = Xstart, y = Ystart;
    unsigned int PixPosX = 0, PixPosY = 0; // 记录读取第几位
    unsigned char Font8bit;                // 八个像素信息（8bit）

    FontInformation myFont; // 字体信息
#if From_Bin
    myFont = FontInfo[filename]; // 获取字体信息
#else
    WordInfoFile.seek(0);                             // 回到文件开头
    WordInfoFile.seek(sizeof(FontInformation) * filename); // 偏移到字体信息位置
    for (int i = 0; i < sizeof(FontInformation); ++i)
    {
        ((char *)&myFont)[i] = WordInfoFile.read(); // 逐一字节读取数据
    }
#endif

    WordImgFile.seek(0);               // 回到文件开头
    WordImgFile.seek(myFont.Deviation); // 偏移到位图数据开头
    // 计算位图大小（字节）
    unsigned int Size = ((myFont.x * myFont.y) / 8) + (((myFont.x * myFont.y) % 8) != 0 ? 1 : 0);
    // 将内容逐一渲染到屏幕上
    for (int i = 0; i < Size; ++i)
    {
        Font8bit = WordImgFile.read();
        for (int ij = 0; ij < 8; ++ij)
        {
            if (Font8bit & 0x80)
            {
                Paint_SetPixel_Gai(y, x + myFont.Dy, BLACK);
            }
            else
            {
                Paint_SetPixel_Gai(y, x + myFont.Dy, WHITE);
            }
            Font8bit <<= 1;
            ++PixPosY;
            ++y;
            // 到边缘换行
            if (PixPosY >= myFont.x)
            {
                ++PixPosX;
                // 到边缘 说明显示完毕了
                if (PixPosX >= myFont.y)
                {
                    return myFont.x;
                }
                PixPosY = 0;
                ++x;
                y = Ystart;
            }
        }
    }
    return myFont.x;
}

void CN_Show(UWORD Xstart, UWORD Ystart, const char *filename)
{
    const char *p_text = filename; // 字符串指针
    unsigned short FontIndex;      // 字体索引

    int PosX = Xstart, PosY = Ystart;
    unsigned int Deflection;   // 字体偏移
    unsigned char LineNum = 0; // 当前几行文字了（从零开始）

#if From_Bin
    WordInfoFile = SPIFFS.open("/FontInfo.bin");
    if (!WordInfoFile)
    {
        Serial.println("Failed to open FontInfo for writing\n");
        return;
    }
#endif
    WordImgFile = SPIFFS.open("/FontData.bin");
    if (!WordImgFile)
    {
        Serial.println("Failed to open FontData for writing\n");
        return;
    }
    while (*p_text != 0)
    {
        FontIndex = from_bytes(p_text); // 计算下一个字的索引值
        p_text += fromDeviation;        // 移动到下一个字开头
        // 显示字,获取字体宽度（用于偏移下一个字的位置）
        Deflection = CN_UTF8_Show(PosY, PosX, FontIndex);
        // 不同情况位移距离
        if (Deflection == 0)
        {
            Deflection = 10; // 空格
        }
        else if (Deflection <= 5)
        {
            Deflection += 4; // 符号加距离（但是部分英文会出问题，后面专门重写实现）
        }
        PosX += (Deflection + 2);
        // 到屏幕边缘了，换行
        if (PosX > 230)
        {
            PosY += 24;
            PosX = 0;
            ++LineNum;
            // 已经显示完整个屏幕了
            if (LineNum >= 4)
            {
                return;
            }
            // 不足显示字体的高度
            if (PosY > 110)
            {
                return;
            }
        }
    }
    // 关闭文件
#if From_Bin
    WordInfoFile.close();
#endif
    WordImgFile.close();
}

void Num_Show(UWORD Xpoint, UWORD Ypoint, int32_t Nummber)
{
    int16_t Num_Bit = 0, Str_Bit = 0;
    uint8_t Str_Array[20] = {0}, Num_Array[20] = {0};
    uint8_t *pStr = Str_Array;

    if (Xpoint > Paint.Width || Ypoint > Paint.Height)
    {
        Debug("Paint_DisNum Input exceeds the normal display range\r\n");
        return;
    }

    // Converts a number to a string
    while (Nummber)
    {
        Num_Array[Num_Bit] = Nummber % 10 + '0';
        Num_Bit++;
        Nummber /= 10;
    }

    // The string is inverted
    while (Num_Bit > 0)
    {
        Str_Array[Str_Bit] = Num_Array[Num_Bit - 1];
        Str_Bit++;
        Num_Bit--;
    }

    // show
    CN_Show(Xpoint, Ypoint, (const char *)pStr);
}

void Img_Show(UWORD Xstart, UWORD Ystart, const unsigned char *img)
{
    unsigned char S = 7;
    unsigned char Z;
    for (int x = 0; x < Xstart; ++x)
    {
        for (int y = 0; y < Ystart; ++y)
        {
            ++S;
            if (S == 8)
            {
                Z = *img;
                ++img;
                S = 0;
            }
            Paint_SetPixel_Gai(y, x, (Z & 0x80) ? WHITE : BLACK);
            Z = Z << 1;
        }
    }
}

#include "qrcodegen.h"
#include "EPD_2in13_V4.h"

uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX];
uint8_t tempBuffer[qrcodegen_BUFFER_LEN_MAX];

// 二维码 适当放大
void Paint_SetBlock(UWORD Xpoint, UWORD Ypoint, UWORD BlockSize, UWORD Color, UWORD Xp)
{
    Xpoint *= BlockSize;
    Ypoint *= BlockSize;
    for (int x = Xpoint; x <= (Xpoint + BlockSize); ++x)
    {
        for (int y = Ypoint; y <= (Ypoint + BlockSize); ++y)
        {
            Paint_SetPixel_Gai(x + Xp, y, Color);
        }
    }
}

void QR(const char *text)
{
    enum qrcodegen_Ecc errCorLvl = qrcodegen_Ecc_LOW;

    bool ok = qrcodegen_encodeText(text, tempBuffer, qrcode, errCorLvl,
                                   qrcodegen_VERSION_MIN, qrcodegen_VERSION_MAX, qrcodegen_Mask_AUTO, true);
    if (!ok)
        return;

    int size = qrcodegen_getSize(qrcode);
    int BlockSize = EPD_2in13_V4_WIDTH / size;
    int bianchuang = 0;
    int XPos = (EPD_2in13_V4_HEIGHT - (size * BlockSize)) / 2;
    for (int x = -bianchuang; x < size + bianchuang; ++x)
    {
        for (int y = -bianchuang; y < size + bianchuang; ++y)
        {
            Paint_SetBlock(x + bianchuang, y + bianchuang, BlockSize, qrcodegen_getModule(qrcode, x, y) ? BLACK : WHITE, XPos);
        }
    }
}


void RenovateScreen(UBYTE *Image){
    UWORD Width, Height;
    Width = (EPD_2in13_V4_WIDTH % 8 == 0)? (EPD_2in13_V4_WIDTH / 8 ): (EPD_2in13_V4_WIDTH / 8 + 1);
    Height = EPD_2in13_V4_HEIGHT;
	
	//Reset
    DEV_Digital_Write(EPD_RST_PIN, 0);
    DEV_Delay_ms(1);
    DEV_Digital_Write(EPD_RST_PIN, 1);

    DEV_Digital_Write(EPD_CS_PIN, 0);// 使能芯片
    DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	DEV_SPI_WriteByte(0x3C); //BorderWavefrom
    DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式
	DEV_SPI_WriteByte(0x80);	

    DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	DEV_SPI_WriteByte(0x01); //Driver output control
    DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式 
	DEV_SPI_WriteByte(0xF9);
	DEV_SPI_WriteByte(0x00);
	DEV_SPI_WriteByte(0x00);
	
    DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	DEV_SPI_WriteByte(0x11); //data entry mode
    DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式 
	DEV_SPI_WriteByte(0x03);




    DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	DEV_SPI_WriteByte(0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
    DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式 
	DEV_SPI_WriteByte(0);
	DEV_SPI_WriteByte(((EPD_2in13_V4_WIDTH-1)>>3) & 0xFF);

    DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	DEV_SPI_WriteByte(0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
    DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式 
	DEV_SPI_WriteByte(0);
    DEV_SPI_WriteByte(0);
    DEV_SPI_WriteByte((EPD_2in13_V4_HEIGHT-1) & 0xFF);
    DEV_SPI_WriteByte(((EPD_2in13_V4_HEIGHT-1) >> 8) & 0xFF);


    DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	DEV_SPI_WriteByte(0x4E); // SET_RAM_X_ADDRESS_COUNTER
    DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式 
	DEV_SPI_WriteByte(0);

    DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	DEV_SPI_WriteByte(0x4F); // SET_RAM_Y_ADDRESS_COUNTER
    DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式 
	DEV_SPI_WriteByte(0);
    DEV_SPI_WriteByte(0);

    DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	DEV_SPI_WriteByte(0x24);   //Write Black and White image to RAM
    DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式 
    for (UWORD j = 0; j < Height; ++j) {
        for (UWORD i = 0; i < Width; ++i) {
			DEV_SPI_WriteByte(Image[i + j * Width]);
		}
	}

    for (size_t i = 0; i < 1; ++i)
    {
      DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	    DEV_SPI_WriteByte(0x22);// Display Update Control
      DEV_Digital_Write(EPD_DC_PIN, 1);// 数据模式
      DEV_SPI_WriteByte(0xff);// fast:0x0c, quality:0x0f, 0xcf
      DEV_Digital_Write(EPD_DC_PIN, 0);// 指令模式
	    DEV_SPI_WriteByte(0x20);// Activate Display Update Sequence

      while(1)
	    {	 //=1 BUSY
		    if(DEV_Digital_Read(EPD_BUSY_PIN)==0) 
			    break;
		    DEV_Delay_ms(10);
	    }
      DEV_Delay_ms(100);
    }
    DEV_Digital_Write(EPD_CS_PIN, 1);// 关闭芯片
}
