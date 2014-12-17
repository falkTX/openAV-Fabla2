
#include "fabla2_ui.hxx"

#include "utils.hxx"
#include "theme.hxx"

#include "header.c"

#include <sstream>

#include "../shared.hxx"
#include "../lv2_messaging.hxx"

// implementation of LV2 Atom writing
#include "writer.hxx"

static void fabla2_widgetCB(Avtk::Widget* w, void* ud);

TestUI::TestUI( PuglNativeWindow parent ):
  Avtk::UI( 780, 330, parent ),
  currentBank( 0 ),
  currentPad( 0 )
{
  themes.push_back( new Avtk::Theme( this, "orange.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "green.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "yellow.avtk" ) );
  themes.push_back( new Avtk::Theme( this, "red.avtk" ) );
  
  // slider vert
  Avtk::Image* headerImage = new Avtk::Image( this, 0, 0, 780, 36, "Header Image" );
  headerImage->load( header.pixel_data );
  
  int s = 32;
  
  bankBtns[0] = new Avtk::Button( this, 5, 43, s, s, "A" );
  
  bankBtns[1] = new Avtk::Button( this, 5 + +s+6, 43, s, s, "B" );
  bankBtns[1]->theme( theme( 1 ) );
  
  bankBtns[2] = new Avtk::Button( this, 5, 43  + +s+6, s, s, "C" );
  bankBtns[2]->theme( theme( 2 ) );
  
  bankBtns[3] = new Avtk::Button( this, 5 + +s+6, 43 +s+6, s, s, "D" );
  bankBtns[3]->theme( theme( 3 ) );
  
  recordOverPad = new Avtk::Button( this, 5, 43+(s+6)*2, s * 2 + 6, s*2+6,  "X-REC" );
  recordOverPad->theme( theme( 4 ) );
  recordOverPad->clickMode( Avtk::Widget::CLICK_TOGGLE );
  
  masterPitch = new Avtk::Dial( this, 5, 43+(s+6)*4+6, s * 2 + 6, s*2+6,  "Pitch" );
  masterPitch->theme( theme( 2 ) );
  
  waveform = new Avtk::Waveform( this, 355, 42, 422, 113, "Waveform" );
  std::vector<float> tmp;
  Avtk::loadSample("/usr/local/lib/lv2/fabla2.lv2/kick.wav", tmp);
  waveform->show( tmp );
  
  // sample edit view
  muteGroup = new Avtk::Button( this, 355, 161, 85, 52, "Mute Group" );
  layers    = new Avtk::Button( this, 355, 218, 85, 109, "Layers" );
  adsr      = new Avtk::Button( this, 446, 161, 59, 166, "ADSR" );
  filt1     = new Avtk::Button( this, 510, 161, 59, 81, "Filter 1" );
  filt2     = new Avtk::Button( this, 510, 246, 59, 81, "Filter 2" );
  bitcrusDist=new Avtk::Button( this, 573, 161, 59, 81, "Bit Cr,Dist" );
  eq        = new Avtk::Button( this, 573, 247, 59, 81, "Equalizer" );
  comp      = new Avtk::Button( this, 635, 161, 59, 81, "Comp" );
  gainPitch = new Avtk::Button( this, 635, 247, 59, 81, "Gain/Ptc" );
  padSends  = new Avtk::Button( this, 699, 161, 32, 166, "Snd" );
  padMaster = new Avtk::Button( this, 736, 160, 40, 166, "Mstr" );
  
  // pads
  int xS = 60;
  int yS = 60;
  int border = 8;
  
  int x = 82;
  int y = -18 + (yS+border) * 4;
  for(int i = 0; i < 16; i++ )
  {
    if( i != 0 && i % 4 == 0 )
    {
      y -= yS + border;
      x = 82;
    }
    
    pads[i] = new Avtk::Button( this, x, y, xS, yS, "-" );
    
    x += xS + border;
  }
  
  // initial values
  bankBtns[0]->value( true );
  
}

void TestUI::setBank( int bank )
{
  // turn off light
  bankBtns[currentBank]->value( false );
  // update
  currentBank = bank;
  // turn on
  bankBtns[currentBank]->value( true );
  
  // pad theme set
  for(int i = 0; i < 16; i++)
    pads[i]->theme( theme( bank ) );
  
  redraw();
}

void TestUI::writeAtom( int eventURI, float value )
{
  printf("Fabla2:UI writeAtom %i, %f\n", eventURI, value );
#define OBJ_BUF_SIZE 1024
	uint8_t obj_buf[OBJ_BUF_SIZE];
	lv2_atom_forge_set_buffer(&forge, obj_buf, OBJ_BUF_SIZE);
  
  LV2_Atom_Forge_Frame frame;
  lv2_atom_forge_frame_time( &forge, 0 );
  LV2_Atom* msg = (LV2_Atom*)lv2_atom_forge_object( &forge, &frame, 0, uris.fabla2_PadPlay );
  
  // Add UI state as properties
  lv2_atom_forge_key(&forge, uris.fabla2_bank);
  lv2_atom_forge_int(&forge, currentBank );
  /*
  lv2_atom_forge_key(&forge, uris.fabla2_pad);
  lv2_atom_forge_int(&forge, currentPad );
  
  lv2_atom_forge_key(&forge, uris.fabla2_velocity);
  lv2_atom_forge_int(&forge, 0 );
  */
  
  lv2_atom_forge_pop(&forge, &frame);
  
  printf("Lv2Atom MSG: size = %li, eventTransfer = %i\n", (long)lv2_atom_total_size(msg), uris.atom_eventTransfer ); 
  write_function( controller, Fabla2::ATOM_IN, lv2_atom_total_size(msg), uris.atom_eventTransfer, &msg );
  
  /*
   // OLD CODE, probably useless
    fabla2_setBank( this, 3 );
    const char* f = "/usr/local/lib/lv2/fabla2.lv2/drum_loop.wav";
    LV2_Atom* msg = fabla2_writeSampleLoadUnload( &forge, &uris, true, f, strlen(f) );
    printf("Lv2Atom MSG: size = %li, eventTransfer = %i\n", (long)lv2_atom_total_size(msg), uris.atom_eventTransfer ); 
    write_function( controller, Fabla2::ATOM_IN, lv2_atom_total_size(msg), uris.atom_eventTransfer, &msg );
  */
}

void TestUI::widgetValueCB( Avtk::Widget* w)
{
  float tmp = w->value();
  
  //printf("widgetCB : %s\n", w->label() );
  
  if( w == recordOverPad )
  {
    write_function( controller, Fabla2::RECORD_OVER_LAST_PLAYED_PAD, sizeof(float), 0, &tmp );
  }
  else if( w == masterPitch )
  {
    float scaleVal = tmp * 24 - 12;
    write_function( controller, Fabla2::MASTER_PITCH, sizeof(float), 0, &scaleVal );
  }
  else if( w == masterVolume )
  {
    write_function( controller, Fabla2::MASTER_VOL, sizeof(float), 0, &tmp );
  }
  else if( w == loadSampleBtn )
  {
    printf("load clicked\n");
    
#define OBJ_BUF_SIZE 1024
    uint8_t obj_buf[OBJ_BUF_SIZE];
    lv2_atom_forge_set_buffer(&forge, obj_buf, OBJ_BUF_SIZE);
    
    std::string filenameToLoad = "test.wav";
    
    LV2_Atom* msg = writeSetFile(&forge, &uris, filenameToLoad );
    
    write_function(controller, 0, lv2_atom_total_size(msg),
              uris.atom_eventTransfer,
              msg);
  }
  else
  {
    // check bank buttons
    for(int i = 0; i < 4; i++)
    {
      if( w == bankBtns[i] )
      {
        setBank( i );
        return;
      }
    }
    
    // check all the pads
    for(int i = 0; i < 16; i++)
    {
      if( w == pads[i] )
      {
        writeAtom( uris.fabla2_PadPlay, w->value() );
        return;
      }
    }
  }
}
