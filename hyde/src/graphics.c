#include <string.h>
#include <stdlib.h>
#include "sprite.h"
#include "graphics.h"
#include "mouse.h"

SDL_Surface *screen; /*pointer to the draw buffer*/
SDL_Surface *buffer; /*pointer to the background image buffer*/
SDL_Surface *videobuffer; /*pointer to the actual video surface*/
SDL_Rect Camera; /*x & y are the coordinates for the background map, w and h are of the screen*/

Uint32 NOW;					/*the current time since program started*/

/*some data on the video settings that can be useful for a lot of functions*/
Uint32 rmask,gmask,bmask,amask;
ScreenData  S_Data;


void Init_Graphics()
{
    Uint32 Vflags = SDL_ANYFORMAT; //setting both flags on (or sets them both true)
    Uint32 HWflag = 0;
    SDL_Surface *temp;
    S_Data.xres = 1024;
    S_Data.yres = 768;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN //interprets bytes backwards
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
    #else
    rmask = 0x000000ff; //red
    gmask = 0x0000ff00; //green
    bmask = 0x00ff0000; //blue
    amask = 0xff000000; //alpha
    #endif
    if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO|SDL_DOUBLEBUF) < 0 ) //i want audio, video, and double buffering, if not...
    {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);
        if(SDL_VideoModeOK(1024, 768, 32, SDL_ANYFORMAT | SDL_HWSURFACE)) // if i can do 1024x768 with 32-bit
    {
        S_Data.xres = 1024;
        S_Data.yres = 768;
        S_Data.depth = 32;
        Vflags =  SDL_ANYFORMAT | SDL_HWSURFACE;
        HWflag = SDL_HWSURFACE;
    }
    else if(SDL_VideoModeOK(1024, 768, 16, SDL_ANYFORMAT | SDL_HWSURFACE))
    {
        S_Data.xres = 1024;
        S_Data.yres = 768;
        S_Data.depth = 16;
        Vflags =  SDL_ANYFORMAT | SDL_HWSURFACE;
        HWflag = SDL_HWSURFACE;
    }
    else if(SDL_VideoModeOK(1024, 768, 16,SDL_ANYFORMAT))
    {
        S_Data.xres = 1024;
        S_Data.yres = 768;
        S_Data.depth = 16;
        Vflags =  SDL_ANYFORMAT;
        HWflag = SDL_SWSURFACE;
    }
    videobuffer = SDL_SetVideoMode(S_Data.xres, S_Data.yres,S_Data.depth, Vflags);
    if ( videobuffer == NULL )
    {
        fprintf(stderr, "Unable to set 1024x768 video: %s\n", SDL_GetError());
        exit(1);
    }
    temp = SDL_CreateRGBSurface(SDL_HWSURFACE, S_Data.xres, S_Data.yres, S_Data.depth,rmask, gmask,bmask,amask); //temporary buffer
    if(temp == NULL)
	  {
        fprintf(stderr,"Couldn't initialize background buffer: %s\n", SDL_GetError());
        exit(1);
	  }
    /* Just to make sure that the surface we create is compatible with the screen*/
    screen = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    temp = SDL_CreateRGBSurface(Vflags, 2048, 768, S_Data.depth,rmask, gmask,bmask,amask);
    if(temp == NULL)
	  {
        fprintf(stderr,"Couldn't initialize Video buffer: %s\n", SDL_GetError());
        exit(1);
	  }
    buffer = SDL_DisplayFormat(temp);
    SDL_FreeSurface(temp);
    Camera.x = 0;
    Camera.y = 0;
    Camera.w = screen->w;/*we want to make sure that our camera is the same size of the video screen*/
    Camera.h = screen->h;
    SDL_ShowCursor(SDL_DISABLE);/*don't show the mouse */
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);
	//IMG_Init(IMG_INIT_PNG);
}


void ResetBuffer()
{
    SDL_BlitSurface(buffer,&Camera,screen,NULL);//blit = block infromation transfer (COPY, PASTE) first two where we are compying from, then what we are compying to
}

void NextFrame()
{
  Uint32 Then;
  SDL_BlitSurface(screen,NULL,videobuffer,NULL);/*copy everything we did to the video surface*/
  SDL_Flip(videobuffer);							/*and then update the screen*/
  Then = NOW;									/*these next few lines  are used to show how long each frame takes to update.  */
  NOW = SDL_GetTicks();
/*  fprintf(stdout,"Ticks passed this frame: %i\n", NOW - Then);*/
  FrameDelay(33); /*this will make your frame rate about 30 frames per second.  If you want 60 fps then set it to about 15 or 16*/
  //will take at least 33 milliseconds to
}

/*
	This will draw a pixel on the surface that is past at the x and y coordinates of the color given;
*/

void DrawPixel(SDL_Surface *screen, Uint8 R, Uint8 G, Uint8 B, int x, int y)
{
    Uint32 color = SDL_MapRGB(screen->format, R, G, B);

    if ( SDL_MUSTLOCK(screen) )
    {
        if ( SDL_LockSurface(screen) < 0 )
        {
            return;
        }
    }
    switch (screen->format->BytesPerPixel)
    {
        case 1:
        { /* Assuming 8-bpp */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *bufp = color;
        }
        break;

        case 2:
        { /* Probably 15-bpp or 16-bpp */
            Uint16 *bufp;

            bufp = (Uint16 *)screen->pixels + y*screen->pitch/2 + x;
            *bufp = color;
        }
        break;

        case 3:
        { /* Slow 24-bpp mode, usually not used */
            Uint8 *bufp;

            bufp = (Uint8 *)screen->pixels + y*screen->pitch + x;
            *(bufp+screen->format->Rshift/8) = R;
            *(bufp+screen->format->Gshift/8) = G;
            *(bufp+screen->format->Bshift/8) = B;
        }
        break;

        case 4:
        { /* Probably 32-bpp */
            Uint32 *bufp;

            bufp = (Uint32 *)screen->pixels + y*screen->pitch/4 + x;
            *bufp = color;
        }
        break;
    }
    if ( SDL_MUSTLOCK(screen) )
    {
        SDL_UnlockSurface(screen);
    }
    SDL_UpdateRect(screen, x, y, 1, 1);
}


Uint32 getpixel(SDL_Surface *surface, int x, int y)
{
    /* Here p is the address to the pixel we want to retrieve*/
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;

    switch(surface->format->BytesPerPixel)
    {
    case 1:
        return *p;

    case 2:
        return *(Uint16 *)p;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
            return p[0] << 16 | p[1] << 8 | p[2];
        else
            return p[0] | p[1] << 8 | p[2] << 16;

    case 4:
        return *(Uint32 *)p;

    default:
        return 0;       /*shouldn't happen, but avoids warnings*/
    }
}



/*
 * Set the pixel at (x, y) to the given value
 * NOTE: The surface must be locked before calling this!
 */
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * surface->format->BytesPerPixel;

    switch(surface->format->BytesPerPixel)
    {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}


/*
  makes sure a minimum number of ticks is waited between frames
  this is to ensure that on faster machines the game won't move so fast that
  it will look terrible.
  This is a very handy function in game programming.
*/

void FrameDelay(Uint32 delay) // <<<MIDTERM
{
    static Uint32 pass = 100; //static exists outside funciton in memory, not changed, maintains value, survives multiple calls, int = machine word size, optimatized for cimouter bits(32-bit)
    Uint32 dif;
    dif = SDL_GetTicks() - pass; //how many frames that have passed
    if(dif< delay)SDL_Delay( delay - dif); //if the dif is lower than the delay, it delays
    pass = SDL_GetTicks();
}
//SDL HWSURFACE PREFERRED
//double buffering -- second copy of orig. screen, draw everything in order, take entire image copy fast to screen, swapping between buffers
/*sets an sdl surface to all color.*/

void BlankScreen(SDL_Surface *buf,Uint32 color)
{
    SDL_LockSurface(buf);
    memset(buf->pixels, (Uint8)color,buf->format->BytesPerPixel * buf->w *buf->h);
    SDL_UnlockSurface(buf);
}
/*
 * This is the beginning of my Palette swapping scheme.  It checks the value
 * of the color it is given to see if the given color is PURE red, PURE green,
 * or PURE blue.  If it is, it takes the value as a percentage to apply to
 * the new color.  It returns either the old color untouched (if it wasn't a 
 * special case) or the new color.
 */

Uint32 SetColor(Uint32 color, int newcolor1,int newcolor2, int newcolor3)
{
    Uint8 r,g,b;
    Uint8 intensity;
    int newcolor;
    SDL_GetRGB(color, screen->format, &r, &g, &b);
    if((r == 0) && (g == 0)&&(b !=0))
    {
        intensity = b;
        newcolor = newcolor3;
    }
    else if((r ==0)&&(b == 0)&&(g != 0))
    {
        intensity = g;
        newcolor = newcolor2;
    }
    else if((g == 0)&&(b == 0)&&(r != 0))
    {
        intensity = r;
        newcolor = newcolor1;
    }
    else return color;
    switch(newcolor)
    {
        case Red:
            r = intensity;
            g = 0;
            b = 0;
            break;
        case Green:
            r = 0;
            g = intensity;
            b = 0;
            break;
        case Blue:
            r = 0;
            g = 0;
            b = intensity;
            break;
        case Yellow:
            r = (Uint8)intensity * 0.7;
            g = (Uint8)intensity * 0.7;
            b = 0;
            break;
        case Orange:
            r = (Uint8)intensity * 0.9;
            g = (Uint8)intensity * 0.4;
            b = (Uint8)intensity * 0.1;
            break;
        case Violet:
            r = (Uint8)intensity * 0.7;
            g = 0;
            b = (Uint8)intensity * 0.7;
            break;
        case Brown:
            r = (Uint8)intensity * 0.6;
            g = (Uint8)intensity * 0.3;
            b = (Uint8)intensity * 0.15;
            break;
        case Grey:
            r = (Uint8)intensity * 0.5;
            g = (Uint8)intensity * 0.5;
            b = (Uint8)intensity * 0.5;
            break;
        case DarkRed:
            r = (Uint8)intensity * 0.5;
            g = 0;
            b = 0;
            break;
        case DarkGreen:
            r = 0;
            g = (Uint8)intensity * 0.5;
            b = 0;
            break;
        case DarkBlue:
            r = 0;
            g = 0;
            b = (Uint8)intensity * 0.5;
            break;
        case DarkYellow:
            r = (Uint8)intensity * 0.4;
            g = (Uint8)intensity * 0.4;
            b = 0;
            break;
        case DarkOrange:
            r = (Uint8)intensity * 0.6;
            g = (Uint8)intensity * 0.2;
            b = (Uint8)intensity * 0.1;
            break;
        case DarkViolet:
            r = (Uint8)intensity * 0.4;
            g = 0;
            b = (Uint8)intensity * 0.4;
            break;
        case DarkBrown:
            r = (Uint8)intensity * 0.2;
            g = (Uint8)intensity * 0.1;
            b = (Uint8)intensity * 0.05;
            break;
        case DarkGrey:
            r = (Uint8)intensity * 0.3;
            g = (Uint8)intensity * 0.3;
            b = (Uint8)intensity * 0.3;
            break;
        case LightRed:
            r = intensity;
            g = (Uint8)intensity * 0.45;
            b = (Uint8)intensity * 0.45;
            break;
        case LightGreen:
            r = (Uint8)intensity * 0.45;
            g = intensity;
            b = (Uint8)intensity * 0.45;
            break;
        case LightBlue:
            r = (Uint8)intensity * 0.45;
            b = intensity;
            g = (Uint8)intensity * 0.45;
            break;
        case LightYellow:
            r = intensity;
            g = intensity;
            b = (Uint8)intensity * 0.45;
            break;
        case LightOrange:
            r = intensity;
            g = (Uint8)intensity * 0.75;
            b = (Uint8)intensity * 0.35;
            break;
        case LightViolet:
            r = intensity;
            g = (Uint8)intensity * 0.45;
            b = intensity;
            break;
        case LightBrown:
            r = intensity;
            g = (Uint8)intensity * 0.85;
            b = (Uint8)intensity * 0.45;
            break;
        case LightGrey:
            r = (Uint8)intensity * 0.85;
            g = (Uint8)intensity * 0.85;
            b = (Uint8)intensity * 0.85;
            break;
        case Black:
            r = (Uint8)intensity * 0.15;
            g = (Uint8)intensity * 0.15;
            b = (Uint8)intensity * 0.15;
            break;
        case White:
            r = intensity;
            g = intensity;
            b = intensity;
            break;
        case Tan:
            r = intensity;
            g = (Uint8)intensity * 0.9;
            b = (Uint8)intensity * 0.6;
            break;
        case Gold:
            r = (Uint8)intensity * 0.8;
            g = (Uint8)intensity * 0.7;
            b = (Uint8)intensity * 0.2;
            break;
        case Silver:
            r = (Uint8)intensity * 0.95;
            g = (Uint8)intensity * 0.95;
            b = intensity;
            break;
        case YellowGreen:
            r = (Uint8)intensity * 0.45;
            g = (Uint8)intensity * 0.75;
            b = (Uint8)intensity * 0.2;
            break;
        case Cyan:
            r = 0;
            g = (Uint8)intensity * 0.85;
            b = (Uint8)intensity * 0.85;
            break;
        case Magenta:
            r = (Uint8)intensity * 0.7;
            g = 0;
            b = (Uint8)intensity * 0.7;
            break;
    }
    return SDL_MapRGB(screen->format,r,g,b);
}

/* This will probably never have to be called, returns the hex code for the
 * enumerated color
 */

Uint32 IndexColor(int color)
{
    switch(color)
    {
    case Red:
        return Red_;
    case Green:
        return Green_;
    case Blue:
        return Blue_;
    case Yellow:
        return Yellow_;
    case Orange:
        return Orange_;
    case Violet:
        return Violet_;
    case Brown:
        return Brown_;
    case Grey:
        return Grey_;
    case DarkRed:
        return DarkRed_;
    case DarkGreen:
        return DarkGreen_;
    case DarkBlue:
        return DarkBlue_;
    case DarkYellow:
        return DarkYellow_;
    case DarkOrange:
        return DarkOrange_;
    case DarkViolet:
        return DarkViolet_;
    case DarkBrown:
        return DarkBrown_;
    case DarkGrey:
        return DarkGrey_;
    case LightRed:
        return LightRed_;
    case LightGreen:
        return LightGreen_;
    case LightBlue:
        return LightBlue_;
    case LightYellow:
        return LightYellow_;
    case LightOrange:
        return LightOrange_;
    case LightViolet:
        return LightViolet_;
    case LightBrown:
        return LightBrown_;
    case LightGrey:
        return LightGrey_;
    case Black:
        return Black_;
    case White:
        return White_;
    case Tan:
        return Tan_;
    case Gold:
        return Gold_;
    case Silver:
        return Silver_;
    case YellowGreen:
        return YellowGreen_;
    case Cyan:
        return Cyan_;
    case Magenta:
        return Magenta_;
    }
    return Black_;
}
