#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include "cinder/emscripten/CinderEmscripten.h"
#include <memory>

EM_JS(void,load_gui,(),{
  
  window.gui = new dat.GUI();

  window.settings = {
    A1:1,
	B1:1,
	M1:4,
	N1:1,
	N2:1,
	N3:2,
	A2:1,
	B2:1,
	M2:6,
	N12:30,
	N22:2,
	N32:2.5,
	Subdivisions:100,
	Checkerboard:7,
	drawNormals:false,
	normalsLength:0.2
  };
   // remember to call "load_gui" in your c++
});

typedef std::shared_ptr<class Gui> GuiRef;

/**
 * A simple class to handle parameters for the simulation. 
 */
class Gui {
    public:
        Gui():gui(emscripten::val::undefined()),
        settings(emscripten::val::undefined())
        {            
            load_gui();

            gui = val::global( "gui" );
            settings = val::global( "settings" );

            gui.call<void>( "add",settings,val( "A1" ),val( 0 ),val( 5 ), val( 0.05f ) );
            gui.call<void>( "add",settings,val( "B1" ),val( 0 ),val( 5 ), val( 0.05f ) );
            gui.call<void>( "add",settings,val( "M1" ),val( 0 ),val( 20 ), val( 0.25f ) );
            gui.call<void>( "add",settings,val( "N1" ),val( 0 ),val( 100 ), val( 1.0f ) );
            gui.call<void>( "add",settings,val( "N2" ),val( -50 ),val( 100 ), val( 0.5f ) );
            gui.call<void>( "add",settings,val( "N3" ),val( -50 ),val( 100 ), val( 0.5f ) );

            gui.call<void>( "add",settings,val( "A2" ),val( 0 ),val( 5 ), val( 0.05f ) );
            gui.call<void>( "add",settings,val( "B2" ),val( 0 ),val( 5 ), val( 0.05f ) );
            gui.call<void>( "add",settings,val( "M2" ),val( 0 ),val( 20 ), val( 0.25f ) );
            gui.call<void>( "add",settings,val( "N12" ),val( 0 ),val( 100 ), val( 1.0f ) );
            gui.call<void>( "add",settings,val( "N22" ),val( -50 ),val( 100 ), val( 0.5f ) );
            gui.call<void>( "add",settings,val( "N32" ),val( -50 ),val( 100 ), val( 0.5f ) );
        }

    static GuiRef create(){
        return GuiRef(new Gui);
    }
    
    void setupGeometryChange( std::function<void( emscripten::val )> func )
    {
        auto controller = gui.call<val>( "add",settings,val( "Subdivisions" ),val( 5 ),val( 500 ), val( 30.5f ) );
        controller.call<void>( "onChange",helpers::generateCallback( func ) );
    }

    
    void updateGUI(){
	   
       /*
        mFormulaParams.mA1 = settings[ "A1" ].as<int>();
	    mFormulaParams.mB1 = settings[ "B1" ].as<int>();
	    mFormulaParams.mM1 = settings[ "M1" ].as<int>();
	    mFormulaParams.mN11 = settings[ "N1" ].as<int>();
	    mFormulaParams.mN21 = settings["N2"].as<int>();
	    mFormulaParams.mN31 = settings["N3"].as<int>();
    
	    mFormulaParams.mA2 = settings[ "A2" ].as<int>();
	    mFormulaParams.mB2 = settings[ "B2" ].as<int>();
	    mFormulaParams.mM2 = settings[ "M2" ].as<int>();
	    mFormulaParams.mN12 = settings[ "N12" ].as<int>();
	    mFormulaParams.mN22 = settings["N22"].as<int>();
	    mFormulaParams.mN32 = settings["N32"].as<int>();
        */
    }


    private:
        emscripten::val gui;
        emscripten::val settings;
};