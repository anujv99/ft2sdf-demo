
#include <freetype/ftmodapi.h>

#include "ftcommon.h"
#include "common.h"
#include "mlgetopt.h"

#include <stdio.h>

  typedef FT_Vector  Vec2;
  typedef FT_BBox    Box;

  #define FT_CALL(X)\
    error = X;\
    if (error != FT_Err_Ok) {\
        printf("FreeType error: %s [LINE: %d, FILE: %s]",\
          FT_Error_String(error), __LINE__, __FILE__);\
        goto Exit;\
    }

  typedef struct  Status_
  {
    FT_Face   face;

    FT_Int    ptsize;

    FT_Int    glyph_index;

    FT_Int    scale;

    FT_Int    spread;

    char*     header;

  } Status;

  static FTDemo_Handle*   handle   = NULL;
  static FTDemo_Display*  display  = NULL;

  static Status status = { NULL, 64, 0, 2, 8, NULL };

  static void
  write_header()
  {
    static char header_string[512];


    sprintf( header_string, "Glyph Index: %d, Pt Size: %d, Spread: %d, Scale: %d",
             status.glyph_index, status.ptsize, status.spread, status.scale );
    status.header = header_string;
  }

  static FT_Error
  event_font_update()
  {
    FT_Error  error = FT_Err_Ok;
    

    FT_CALL( FT_Property_Set( handle->library, "sdf", "spread", &status.spread ) );

    FT_CALL( FT_Set_Pixel_Sizes( status.face, 0, status.ptsize ) );
    FT_CALL( FT_Load_Glyph( status.face, status.glyph_index, FT_LOAD_DEFAULT ) );
    FT_CALL( FT_Render_Glyph( status.face->glyph, FT_RENDER_MODE_SDF ) );

  Exit:
    return error;
  }

  static void
  event_color_change()
  {
    static int     i = 0;
    unsigned char  r = i & 2 ? 0xff : 0;
    unsigned char  g = i & 4 ? 0xff : 0;
    unsigned char  b = i & 1 ? 0xff : 0;


    display->back_color = grFindColor( display->bitmap,  r,  g,  b, 0xff );
    display->fore_color = grFindColor( display->bitmap, ~r, ~g, ~b, 0xff );
    display->warn_color = grFindColor( display->bitmap,  r, ~g, ~b, 0xff );

    i++;
  }

  static int
  Process_Event()
  {
    grEvent  event;
    int      ret = 0;

    grListenSurface( display->surface, 0, &event );

    switch (event.key) {
    case grKEY( 'q' ):
    case grKeyEsc:
      ret = 1;
      break;
    case grKEY( 'z' ):
      status.scale++;
      break;
    case grKEY( 'x' ):
      status.scale--;
      if ( status.scale < 1 )
        status.scale = 1;
      break;
    case grKeyPageUp:
      status.ptsize += 24;
    case grKeyUp:
      status.ptsize++;
      if ( status.ptsize > 512 )
        status.ptsize = 512;
      event_font_update();
      break;
    case grKeyPageDown:
      status.ptsize -= 24;
    case grKeyDown:
      status.ptsize--;
      if ( status.ptsize < 8 )
        status.ptsize = 8;
      event_font_update();
      break;
    case grKEY( 'w' ):
      status.spread++;
      if ( status.spread > 32 )
        status.spread = 32;
      event_font_update();
      break;
    case grKEY( 's' ):
      status.spread--;
      if ( status.spread < 2 )
        status.spread = 2;
      event_font_update();
      break;
    case grKeyRight:
      status.glyph_index++;
      event_font_update();
      break;
    case grKeyLeft:
      status.glyph_index--;
      event_font_update();
      break;
    default:
        break;
    }

    return ret;
  }

  static FT_Error
  draw()
  {
    FT_Bitmap*  bitmap = &status.face->glyph->bitmap;
    Box         draw_region;
    Box         sample_region;
    Vec2        center;
    FT_Short*   buffer;


    if ( !bitmap || !bitmap->buffer )
      return FT_Err_Invalid_Argument;

    center.x = display->bitmap->width / 2;
    center.y = display->bitmap->rows  / 2;

    draw_region.xMin = center.x - ( bitmap->width * status.scale) / 2;
    draw_region.xMax = center.x + ( bitmap->width * status.scale) / 2;
    draw_region.yMin = center.y - ( bitmap->rows  * status.scale) / 2;
    draw_region.yMax = center.y + ( bitmap->rows  * status.scale) / 2;

    sample_region.xMin = 0;
    sample_region.xMax = bitmap->width * status.scale;
    sample_region.yMin = 0;
    sample_region.yMax = bitmap->rows * status.scale;

    if ( draw_region.yMin < 0 )
    {
      sample_region.yMax -= draw_region.yMin;
      draw_region.yMin = 0;
    }

    if ( draw_region.yMax > display->bitmap->rows )
    {
      sample_region.yMin += draw_region.yMax - display->bitmap->rows;
      draw_region.yMax = display->bitmap->rows;
    }

    if ( draw_region.xMin < 0 )
    {
      sample_region.xMin -= draw_region.xMin;
      draw_region.xMin = 0;
    }

    if ( draw_region.xMax > display->bitmap->width )
    {
      sample_region.xMax += draw_region.xMax - display->bitmap->width;
      draw_region.xMax = display->bitmap->width;
    }

    buffer = (FT_Short*)bitmap->buffer;

    for ( FT_Int j = draw_region.yMax - 1, y = sample_region.yMin; j >= draw_region.yMin; j--, y++ )
    {
      for ( FT_Int i = draw_region.xMin, x = sample_region.xMin; i < draw_region.xMax; i++, x++ )
      {
        FT_UInt   display_index = j * display->bitmap->width + i;
        FT_UInt   bitmap_index  = ( y / status.scale ) * bitmap->width + ( x / status.scale );
        FT_Short  pixel_value   = buffer[bitmap_index];


        pixel_value = pixel_value < 0 ? -pixel_value : pixel_value;
        pixel_value /= ( 4 * status.spread );
        pixel_value = 255 - pixel_value;

        if ( pixel_value < 0 ) pixel_value = 0;
        if ( pixel_value > 255 ) pixel_value = 255;

        display_index *= 3;
        display->bitmap->buffer[display_index + 0] = pixel_value;
        display->bitmap->buffer[display_index + 1] = pixel_value;
        display->bitmap->buffer[display_index + 2] = pixel_value;
      }
    }

    return FT_Err_Ok;
  }

  int
  main( int     argc,
        char**  argv )
  {
    FT_Error  error = FT_Err_Ok;


    if ( argc != 3 )
    {
      printf( "Usage: [ptsize] [font file]\n" );
      exit( -1 );
    }

    status.ptsize = atoi( argv[1] );
    handle = FTDemo_New();

    if ( !handle )
    {
      printf( "Failed to create FTDemo_Handle\n" );
      goto Exit;
    }

    display = FTDemo_Display_New( NULL, "800x600" );

    if ( !display )
    {
      printf( "Failed to create FTDemo_Display\n" );
      goto Exit;
    }

    grSetTitle( display->surface, "Signed Distance Field Viewer" );
    event_color_change();

    FT_CALL( FT_New_Face( handle->library, argv[2], 0, &status.face ) );
    FT_CALL( event_font_update() );

    do 
    {
      FTDemo_Display_Clear( display );

      draw();

      write_header();

      if ( status.header )
        grWriteCellString( display->bitmap, 0, 0, status.header, display->fore_color );

      grRefreshSurface( display->surface );
    } while ( !Process_Event() );

  Exit:
    if ( status.face )
      FT_Done_Face( status.face );
    if ( display )
      FTDemo_Display_Done( display );
    if ( handle )
      FTDemo_Done( handle );
    exit( error );
  }
