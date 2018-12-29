#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/ImageIo.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"
#include "cinder/GeomIo.h"
#include <emscripten.h>
#include <emscripten/val.h>
#include <emscripten/bind.h>
#include "cinder/emscripten/CinderEmscripten.h"

using namespace ci;
using namespace ci::app;
using namespace emscripten;
using namespace ci::em;


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

class SuperformulaGpuApp : public App {
  public:
	void	setup() override;
	void	resize() override;
	void	update() override;
	void	draw() override;

	void	setupGeometry();
	void updateGUI();
	
	CameraPersp				mCam;
	gl::BatchRef			mBatch, mNormalsBatch;
	gl::GlslProgRef			mGlsl, mNormalsGlsl;
	mat4					mRotation;
#if ! defined( CINDER_GL_ES )
	params::InterfaceGlRef	mParams;
#endif

	bool					mDrawNormals;
	float					mNormalsLength;
	int						mSubdivisions;
	int						mCheckerFrequency;
	
	// This is dependent on the C++ compiler structuring these vars in RAM the same way that GL's std140 does
	struct {
		float	mA1, mA2;
		float	mB1, mB2;
		float	mM1, mM2;
		float	mN11, mN12;
		float	mN21, mN22;
		float	mN31, mN32;
	} mFormulaParams;
	gl::UboRef				mFormulaParamsUbo;

	emscripten::val gui = emscripten::val::undefined();
	emscripten::val settings = emscripten::val::undefined();
};

void SuperformulaGpuApp::setupGeometry()
{
	auto plane = geom::Plane().subdivisions( ivec2( mSubdivisions, mSubdivisions ) );
	mBatch = gl::Batch::create( plane, mGlsl );
	mNormalsBatch = gl::Batch::create( plane >> geom::VertexNormalLines( 0.0f ), mNormalsGlsl );
}

void SuperformulaGpuApp::updateGUI(){
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

	
	
}

void SuperformulaGpuApp::setup()
{
	mFormulaParams.mA1 = mFormulaParams.mA2 = 1.0f;
	mFormulaParams.mB1 = mFormulaParams.mB2 = 1.0f;
	mFormulaParams.mN11 = mFormulaParams.mN12 = 1.0f;
	mFormulaParams.mN21 = mFormulaParams.mN22 = 1.0f;
	mFormulaParams.mM1 = 4.0f;
	mFormulaParams.mM2 = 6.0f;
	mFormulaParams.mN31 = 2.0f;
	mFormulaParams.mN32 = 2.5f;

	mDrawNormals = false;
	mNormalsLength = 0.20f;
	mSubdivisions = 100;
	mCheckerFrequency = 7;

load_gui();

gui = val::global( "gui" );
settings = val::global( "settings" );

gui.call<void>( "add",settings,val( "A1" ),val( 0 ),val( 5 ), val(0.05f) );
gui.call<void>( "add",settings,val( "B1" ),val( 0 ),val( 5 ), val(0.05f) );
gui.call<void>( "add",settings,val( "M1" ),val( 0 ),val( 20 ), val(0.25f) );
gui.call<void>( "add",settings,val( "N1" ),val( 0 ),val( 100 ), val(1.0f) );
gui.call<void>( "add",settings,val( "N2" ),val( -50 ),val( 100 ), val(0.5f) );
gui.call<void>( "add",settings,val( "N3" ),val( -50 ),val( 100 ), val(0.5f) );


gui.call<void>( "add",settings,val( "A2" ),val( 0 ),val( 5 ), val(0.05f) );
gui.call<void>( "add",settings,val( "B2" ),val( 0 ),val( 5 ), val(0.05f) );
gui.call<void>( "add",settings,val( "M2" ),val( 0 ),val( 20 ), val(0.25f) );
gui.call<void>( "add",settings,val( "N12" ),val( 0 ),val( 100 ), val(1.0f) );
gui.call<void>( "add",settings,val( "N22" ),val( -50 ),val( 100 ), val(0.5f) );
gui.call<void>( "add",settings,val( "N32" ),val( -50 ),val( 100 ), val(0.5f) );

auto controller = gui.call<val>( "add",settings,val( "Subdivisions" ),val( 5 ),val( 500 ), val(30.5f) );
std::function<void(emscripten::val)> func = [=](emscripten::val e)->void{
	setupGeometry();
};
controller.call<void>("onChange",helpers::generateCallback(func));


#if ! defined( CINDER_GL_ES )
	mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( ivec2( 200, 400 ) ) );
	mParams->addParam( "A (1)", &mFormulaParams.mA1 ).min( 0 ).max( 5 ).step( 0.05f );
	mParams->addParam( "B (1)", &mFormulaParams.mB1 ).min( 0 ).max( 5 ).step( 0.05f );
	mParams->addParam( "M (1)", &mFormulaParams.mM1 ).min( 0 ).max( 20 ).step( 0.25f );
	mParams->addParam( "N1 (1)", &mFormulaParams.mN11 ).min( 0 ).max( 100 ).step( 1.0f );
	mParams->addParam( "N2 (1)", &mFormulaParams.mN21 ).min( -50 ).max( 100 ).step( 0.5f );
	mParams->addParam( "N3 (1)", &mFormulaParams.mN31 ).min( -50 ).max( 100 ).step( 0.5f );
	mParams->addSeparator();
	mParams->addParam( "A (2)", &mFormulaParams.mA2 ).min( 0 ).max( 5 ).step( 0.05f );
	mParams->addParam( "B (2)", &mFormulaParams.mB2 ).min( 0 ).max( 5 ).step( 0.05f );
	mParams->addParam( "M (2)", &mFormulaParams.mM2 ).min( 0 ).max( 20 ).step( 0.25f );
	mParams->addParam( "N1 (2)", &mFormulaParams.mN12 ).min( 0 ).max( 100 ).step( 1.0f );
	mParams->addParam( "N2 (2)", &mFormulaParams.mN22 ).min( -50 ).max( 100 ).step( 0.5f );
	mParams->addParam( "N3 (2)", &mFormulaParams.mN32 ).min( -50 ).max( 100 ).step( 0.5f );
	mParams->addSeparator();
	mParams->addParam( "Subdivisions", &mSubdivisions ).min( 5 ).max( 500 ).step( 30 ).updateFn( [&] { setupGeometry(); } );
	mParams->addParam( "Checkerboard", &mCheckerFrequency ).min( 1 ).max( 500 ).step( 3 );
	mParams->addSeparator();
	mParams->addParam( "Draw Normals", &mDrawNormals );
	mParams->addParam( "Normals Length", &mNormalsLength ).min( 0.0f ).max( 2.0f ).step( 0.025f );
#endif



	mCam.lookAt( vec3( 3, 2, 4 ) * 0.75f, vec3( 0 ) );

#if defined( CINDER_GL_ES_3 )
	mGlsl = gl::GlslProg::create( loadAsset( "shader_es3.vert" ), loadAsset( "shader_es3.frag" ) );
	mNormalsGlsl = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "normals_shader_es3.vert" ) )
											.fragment( loadAsset( "normals_shader_es3.frag" ) )
											.attrib( geom::CUSTOM_0, "vNormalWeight" ) );
#else
	mGlsl = gl::GlslProg::create( loadAsset( "shader.vert" ), loadAsset( "shader.frag" ) );
	mNormalsGlsl = gl::GlslProg::create( gl::GlslProg::Format().vertex( loadAsset( "normals_shader.vert" ) )
											.fragment( loadAsset( "normals_shader.frag" ) )
											.attrib( geom::CUSTOM_0, "vNormalWeight" ) );
#endif

	// allocate our UBO
	mFormulaParamsUbo = gl::Ubo::create( sizeof( mFormulaParams ), &mFormulaParams, GL_DYNAMIC_DRAW );
	// and bind it to buffer base 0; this is analogous to binding it to texture unit 0
	mFormulaParamsUbo->bindBufferBase( 0 );
	// and finally tell the shaders that their uniform buffer 'FormulaParams' can be found at buffer base 0
	mGlsl->uniformBlock( "FormulaParams", 0 );
	mNormalsGlsl->uniformBlock( "FormulaParams", 0 );

	setupGeometry();

	gl::enableDepthWrite();
	gl::enableDepthRead();
}

void SuperformulaGpuApp::resize()
{
	mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
	gl::setMatrices( mCam );
}

void SuperformulaGpuApp::update()
{
	// Rotate the by 2 degrees around an arbitrary axis
	vec3 axis = vec3( cos( getElapsedSeconds() / 3 ), sin( getElapsedSeconds() / 2 ), sin( getElapsedSeconds() / 5 ) );
	mRotation *= rotate( toRadians( 0.2f ), normalize( axis ) );

	// buffer our data to our UBO to reflect any changed parameters
	mFormulaParamsUbo->bufferSubData( 0, sizeof( mFormulaParams ), &mFormulaParams );
	
	mNormalsBatch->getGlslProg()->uniform( "uNormalsLength", mNormalsLength );
	mBatch->getGlslProg()->uniform( "uCheckerFrequency", mCheckerFrequency );

	updateGUI();
}

void SuperformulaGpuApp::draw()
{
	gl::clear( Color( 0.2f, 0.2f, 0.2f ) );

	gl::setMatrices( mCam );
	gl::pushMatrices();
		gl::multModelMatrix( mRotation );
		mBatch->draw();
		gl::color( 0.25f, 0.5f, 1.0f, 1 );
		if( mDrawNormals )
			mNormalsBatch->draw();
	gl::popMatrices();

#if ! defined( CINDER_GL_ES )
	mParams->draw();
#endif
}

CINDER_APP( SuperformulaGpuApp, RendererGl )
