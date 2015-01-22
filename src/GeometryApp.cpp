#include "cinder/Camera.h"
#include "cinder/GeomIo.h"
#include "cinder/ImageIo.h"
#include "cinder/MayaCamUI.h"
#include "cinder/app/AppNative.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Batch.h"
#include "cinder/gl/Context.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/VboMesh.h"
#include "cinder/params/Params.h"

#include "DebugMesh.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class GeometryApp : public AppNative {
  public:
	typedef enum { CAPSULE, CONE, CUBE, CYLINDER, HELIX, ICOSAHEDRON, ICOSPHERE, SPHERE, TEAPOT, TORUS, PLANE } Primitive;
    typedef enum { PLA,TWIST,SQUASH,SQUASH2,SPH,CUSTOM23,CUSTOM123 } Transformative;
	typedef enum { LOW, DEFAULT, HIGH } Quality;
	typedef enum { SHADED, WIREFRAME } ViewMode;

	void prepareSettings( Settings* settings );
	void setup();
	void update();
	void draw();

	void mouseDown( MouseEvent event );
	void mouseDrag( MouseEvent event );

	void keyDown( KeyEvent event );

	void resize();
  private:
	void createGrid();
	void createPlaneShader();
	void createWireframeShader();
	void createPrimitive();
	void createParams();
    
    void createTwistShader();
    void createSquashShader();
    void createSquashShader2();
    void createSphereShader();
    void createCustomShader23();
    void createCustomShader123();

	void setSubdivision(int subdivision) { mSubdivision = math<int>::clamp(subdivision, 1, 5); createPrimitive(); }
	int  getSubdivision() const { return mSubdivision; }
    
    void setXlim(float x_lim) { xlim = math<float>::clamp(x_lim, 0, 5); }
	int  getXlim() const { return xlim; }
    
    void setYlim(float y_lim) { ylim = math<float>::clamp(y_lim, 0, 5); }
	int  getYlim() const { return ylim; }
    
    void setZlim(float z_lim) { zlim = math<float>::clamp(z_lim, 0, 5); }
	int  getZlim() const { return zlim; }
    
    void setRed(float r_ed) { red = math<float>::clamp(r_ed, 0, 1); }
	int  getRed() const { return red; }
    
    void setGreen(float g_reen) { green = math<float>::clamp(g_reen, 0, 1); }
	int  getGreen() const { return green; }
    
    void setBlue(float b_lue) { blue = math<float>::clamp(b_lue, 0, 1); }
	int  getBlue() const { return blue; }

	void enableColors(bool enabled=true) { mShowColors = enabled; createPrimitive(); }
	bool isColorsEnabled() const { return mShowColors; }

	Primitive			mPrimitiveSelected;
    Transformative      mTransformation;
    Transformative      mTransformationSelected;
	Primitive			mPrimitiveCurrent;
	Quality				mQualitySelected;
	Quality				mQualityCurrent;
	ViewMode			mViewMode;

	int					mSubdivision;
    float               xlim,ylim,zlim;
    float               red,green,blue;

	bool				mShowColors;
	bool				mShowNormals;
	bool				mShowGrid;
    bool                mRotate;
    bool                mRotatexz;
    bool                mTranslate;
    bool                mTranslatexz;

	CameraPersp			mCamera;
	MayaCamUI			mMayaCam;
	bool				mRecenterCamera;
	vec3				mCameraCOI;

	gl::VertBatchRef	mGrid;

	gl::BatchRef		mPrimitive;
	gl::BatchRef		mPrimitiveWireframe;
	gl::BatchRef		mNormals;
    gl::BatchRef		mNormals_to_plane;

	gl::GlslProgRef		mPlaneShader;
    gl::GlslProgRef		mTwistShader;
    gl::GlslProgRef		mSquashShader;
    gl::GlslProgRef		mSquashShader2;
    gl::GlslProgRef		mSphereShader;
    gl::GlslProgRef		mCustomShader23;
    gl::GlslProgRef		mCustomShader123;
	gl::GlslProgRef		mWireframeShader;

	gl::TextureRef		mTexture;
    
    float angle_deg_max =0;
    float height_of_cube = 0;
    bool positive = true;
    bool flag = true;
    bool flagSelected = true;
    float move = 0.0;
	
#if ! defined( CINDER_GL_ES )
	params::InterfaceGlRef	mParams;
#endif
};

void GeometryApp::prepareSettings( Settings* settings )
{
	settings->setWindowSize(1024, 768);
	settings->enableHighDensityDisplay();
}

void GeometryApp::setup()
{
	// Initialize variables.
    mPrimitiveSelected = mPrimitiveCurrent = SPHERE;
    mTransformation = mTransformationSelected = PLA;
	//mPrimitiveSelected = mPrimitiveCurrent = ICOSAHEDRON;
	mQualitySelected = mQualityCurrent = HIGH;
	mViewMode = SHADED;

	mShowColors = false;
	mShowNormals = false;
	mShowGrid = false;
    mRotate = false;
    mRotatexz = false;
    mTranslate = false;
    mTranslatexz = false;

	mSubdivision = 1;
    xlim = 0.01;
    ylim = 2.0;
    zlim = 0.05;

//    xlim = 2.0;
//    ylim = 0.01;
//    zlim = 0.05;

    
    red = 0.0;
    green = 0.9;
    blue = 1.0;
	
	// Load the textures.
	gl::Texture::Format fmt;
	fmt.setAutoInternalFormat();
	fmt.setWrap( GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE );
	mTexture = gl::Texture::create( loadImage( loadAsset("stripes.jpg") ), fmt );

	// Setup the camera.
	mCamera.setEyePoint( normalize( vec3( 3, 3, 6 ) ) * 10.0f );
	
//    mCamera.lookAt(vec3(0,0,5), vec3(0,0,0));
    mCamera.setCenterOfInterestPoint( mCameraCOI );
    
//    mCamera.setPerspective(60, getWindowAspectRatio(), 0.1, 100);
    

	// Load and compile the shaders.
	createPlaneShader();
    createTwistShader();
    createSquashShader();
    createSquashShader2();
    createSphereShader();
    createCustomShader23();
    createCustomShader123();
	createWireframeShader();

	// Create the meshes.
	createGrid();
	createPrimitive();

	// Enable the depth buffer.
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Create a parameter window, so we can toggle stuff.
	createParams();
}

void GeometryApp::update()
{
	// If another primitive or quality was selected, reset the subdivision and recreate the primitive.
	if( mPrimitiveCurrent != mPrimitiveSelected || mQualitySelected != mQualityCurrent ) {
		mSubdivision = 1;
		mPrimitiveCurrent = mPrimitiveSelected;
		mQualityCurrent = mQualitySelected;
		createPrimitive();
	}
    
    if (mTransformation != mTransformationSelected) {
        mTransformation = mTransformationSelected;
        createPrimitive();
    }
    
    if (flag != flagSelected) {
        flag = flagSelected;
        move = 0.0;
    }
    
//    cout << "mTransformation - " << mTransformation <<endl;

	// After creating a new primitive, gradually move the camera to get a good view.
	if( mRecenterCamera ) {
//		float distance = glm::distance( mCamera.getEyePoint(), mCameraCOI );
//		mCamera.setEyePoint( mCameraCOI - lerp( distance, 5.0f, 0.1f ) * mCamera.getViewDirection() );
//		mCamera.setCenterOfInterestPoint( lerp( mCamera.getCenterOfInterestPoint(), mCameraCOI, 0.25f) );
	}
}

void GeometryApp::draw()
{
	// Prepare for drawing.
    gl::clear( Color::black() );
    if (mTransformation == PLA) {
        red = 0.0;
        green = 0.9;
        blue = 1.0;
        gl::clear( Color(0.7,0.4,0.3) );
    }
    
    else if (mTransformation == TWIST) {
        red = 0.9;
        green = 1.0;
        blue = 0.7;
        gl::clear( Color(0.5,0.2,0.7) );
        
        //        red = 0.4;
//        green = 1.0;
//        blue = 0.4;
//        gl::clear( Color(0.7,0.2,0.5) );
    }
    
    else if (mTransformation == SQUASH) {
        red = 0.0;
        green = 0.9;
        blue = 1.0;
        gl::clear( Color(0.7,0.4,0.3) );
        
        //        red = 0.9;
//        green = 1.0;
//        blue = 0.7;
//        gl::clear( Color(0.5,0.2,0.7) );
    }
    
    else if (mTransformation == SQUASH2) {
        red = 0.9;
        green = 1.0;
        blue = 0.7;
        gl::clear( Color(0.5,0.2,0.7) );
    }
    
    
    else if (mTransformation == SPH) {
        red = 0.4;
        green = 1.0;
        blue = 0.4;
        gl::clear( Color(0.7,0.2,0.5) );
    }
    
    else if (mTransformation == CUSTOM23) {
        red = 0.9;
        green = 1.0;
        blue = 0.7;
        
        //        red = 1.0;
//        green = 0.5;
//        blue = 0.9;
        gl::clear( Color(0.4,0.7,0.2) );
    }
    
    
    else if (mTransformation == CUSTOM123) {
        red = 0.0;
        green = 0.9;
        blue = 1.0;
        gl::clear( Color(0.7,0.4,0.3) );
    }
    
//    gl::clear( Color(0.7,0.7,0.6) );
    
	
	mCamera.setCenterOfInterestPoint( mCameraCOI );
    gl::setMatrices( mCamera );
    
    
    float mxAm = 0.05 * (1.0 + sin(getElapsedSeconds()));
    
    
    switch (mTransformation) {
        case PLA:
            mPlaneShader->uniform("mixAmount", mxAm);
            mPlaneShader->uniform("worldUp", mCamera.getWorldUp());
            mPlaneShader->uniform("angle_deg_max", angle_deg_max);
            mPlaneShader->uniform("height", height_of_cube);
            mPlaneShader->uniform("centerPoint", mCameraCOI);
            mPlaneShader->uniform("flag", flag);
            mPlaneShader->uniform("move", move);
            
            
            if (flag == false) {
                if (move < 1.0) {
                    move += 0.01;
                }
            }

            
            
//            if (mxAm > 0.05) {
//                mPlaneShader->uniform("flag", false);
//            }
//            else{
//                mPlaneShader->uniform("flag", true);
//            }
            break;
        case TWIST:
            mTwistShader->uniform("mixAmount", mxAm);
            mTwistShader->uniform("worldUp", mCamera.getWorldUp());
            mTwistShader->uniform("angle_deg_max", angle_deg_max);
            mTwistShader->uniform("height", height_of_cube);
            mTwistShader->uniform("centerPoint", mCameraCOI);
            
            break;
        case SQUASH:
            mSquashShader->uniform("mixAmount", mxAm);
            mSquashShader->uniform("worldUp", mCamera.getWorldUp());
            mSquashShader->uniform("angle_deg_max", angle_deg_max);
            mSquashShader->uniform("height", height_of_cube);
            mSquashShader->uniform("centerPoint", mCameraCOI);
            
            mSquashShader->uniform("xlim", xlim);
            mSquashShader->uniform("ylim", ylim);
            mSquashShader->uniform("zlim", zlim);
            
            break;
        case SQUASH2:
            mSquashShader2->uniform("mixAmount", mxAm);
            mSquashShader2->uniform("worldUp", mCamera.getWorldUp());
            mSquashShader2->uniform("angle_deg_max", angle_deg_max);
            mSquashShader2->uniform("height", height_of_cube);
            mSquashShader2->uniform("centerPoint", mCameraCOI);
            
            mSquashShader2->uniform("xlim", xlim);
            mSquashShader2->uniform("ylim", ylim);
            mSquashShader2->uniform("zlim", zlim);
            
            break;
            
            
        case SPH:
            mSphereShader->uniform("mixAmount", mxAm);
            mSphereShader->uniform("worldUp", mCamera.getWorldUp());
            mSphereShader->uniform("angle_deg_max", angle_deg_max);
            mSphereShader->uniform("height", height_of_cube);
            mSphereShader->uniform("centerPoint", mCameraCOI);
            
            break;
        
        case CUSTOM23:
            mCustomShader23->uniform("mixAmount", mxAm);
            mCustomShader23->uniform("worldUp", mCamera.getWorldUp());
            mCustomShader23->uniform("angle_deg_max", angle_deg_max);
            mCustomShader23->uniform("height", height_of_cube);
            mCustomShader23->uniform("centerPoint", mCameraCOI);
            
            mCustomShader23->uniform("xlim", xlim);
            mCustomShader23->uniform("ylim", ylim);
            mCustomShader23->uniform("zlim", zlim);
            
            break;
            
            
            
        case CUSTOM123:
            mCustomShader123->uniform("mixAmount", mxAm);
            mCustomShader123->uniform("worldUp", mCamera.getWorldUp());
            mCustomShader123->uniform("angle_deg_max", angle_deg_max);
            mCustomShader123->uniform("height", height_of_cube);
            mCustomShader123->uniform("centerPoint", mCameraCOI);
            
            mCustomShader123->uniform("xlim", xlim);
            mCustomShader123->uniform("ylim", ylim);
            mCustomShader123->uniform("zlim", zlim);
            
            break;
            
        default:
            
            break;
    }
    

    
    if (positive) {
        angle_deg_max += 1;
    }
    else{
        angle_deg_max -=1;
    }
    
    if (angle_deg_max > 360) {
        positive = false;
    }
    
    if (angle_deg_max < 0) {
        positive = true;
    }
    
    height_of_cube = 0.9;
    
//    vec3 cam_zoom = vec3(0,0,mxAm * 100);
//    mCamera.setCenterOfInterestPoint(cam_zoom);
	
	// Draw the grid.
	if( mShowGrid && mGrid ) {
		gl::ScopedGlslProg scopedGlslProg( gl::context()->getStockShader( gl::ShaderDef().color() ) );
		mGrid->draw();
	}

	if( mPrimitive ) {
		gl::ScopedTextureBind scopedTextureBind( mTexture );

		// Rotate it slowly around the y-axis.
		gl::pushModelView();
        
        if (mTranslate) {
            gl::translate(vec3(0,-mxAm*50,-mxAm*100));
        }
        
        if (mTranslatexz) {
            gl::translate(vec3(0,sin(-mxAm*50),cos(-mxAm*100)));
        }
        
        
        
        if (mRotate) {
            gl::rotate( float( getElapsedSeconds() / 5 ), 0.0f, 1.0f, 0.0f );
        }
        
        if (mRotatexz) {
            gl::rotate( float( getElapsedSeconds() / 5 ), 0.5f, 0.0f, 0.5f );
        }
        


		// Draw the normals.
		if( mShowNormals && mNormals )
			mNormals->draw();

		// Draw the primitive.
		gl::color( Color(red, green, blue) );
		
		// (If transparent, render the back side first).
		if( mViewMode == WIREFRAME ) {
			gl::enableAlphaBlending();

			gl::enable( GL_CULL_FACE );
			glCullFace( GL_FRONT );

			mWireframeShader->uniform( "uBrightness", 0.5f );
			mPrimitiveWireframe->draw();
		}

		// (Now render the front side.)
		if( mViewMode == WIREFRAME ) {
			glCullFace( GL_BACK );

			mWireframeShader->uniform( "uBrightness", 1.0f );
			mPrimitiveWireframe->draw();
			
			gl::disable( GL_CULL_FACE );

			gl::disableAlphaBlending();
		}
		else
			mPrimitive->draw();
		
		// Done.
		gl::popModelView();
	}

	// Render the parameter window.
#if ! defined( CINDER_GL_ES )
	if( mParams )
		mParams->draw();
#endif
}

void GeometryApp::mouseDown( MouseEvent event )
{
	mRecenterCamera = false;

	mMayaCam.setCurrentCam( mCamera );
	mMayaCam.mouseDown( event.getPos() );
}

void GeometryApp::mouseDrag( MouseEvent event )
{
	mMayaCam.mouseDrag( event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown() );
	mCamera = mMayaCam.getCamera();
}

void GeometryApp::resize(void)
{
	mCamera.setAspectRatio( getWindowAspectRatio() );
	
	if(mWireframeShader)
		mWireframeShader->uniform( "uViewportSize", vec2( getWindowSize() ) );
}

void GeometryApp::keyDown( KeyEvent event )
{
	switch( event.getCode() ) {
		case KeyEvent::KEY_SPACE:
			mPrimitiveSelected = static_cast<Primitive>( static_cast<int>(mPrimitiveSelected) + 1 );
			createPrimitive();
			break;
		case KeyEvent::KEY_c:
			mShowColors = ! mShowColors;
			createPrimitive();
			break;
		case KeyEvent::KEY_n:
			mShowNormals = ! mShowNormals;
			break;
		case KeyEvent::KEY_g:
			mShowGrid = ! mShowGrid;
			break;
		case KeyEvent::KEY_q:
			mQualitySelected = Quality( (int)( mQualitySelected + 1 ) % 3 );
			break;
		case KeyEvent::KEY_w:
			if(mViewMode == WIREFRAME)
				mViewMode = SHADED;
			else
				mViewMode = WIREFRAME;
			break;
		case KeyEvent::KEY_RETURN:
			createPlaneShader();
			createPrimitive();
			break;
	}
}

void GeometryApp::createParams()
{
#if ! defined( CINDER_GL_ES )
	std::string primitives[] = { "Capsule", "Cone", "Cube", "Cylinder", "Helix", "Icosahedron", "Icosphere", "Sphere", "Teapot", "Torus", "Plane" };
    std::string transformation[] = { "Plane","Twist","Squash","Squash2","Sphere","Custom23","Custom123" };
	std::string qualities[] = { "Low", "Default", "High" };
//	std::string viewmodes[] = { "Shaded", "Wireframe" };

	mParams = params::InterfaceGl::create( getWindow(), "Transformations", ivec2( 340, 200 ) );
	mParams->setOptions( "", "valueswidth=100 refresh=0.1" );

	mParams->addParam( "Original", vector<string>(primitives,primitives+11), (int*) &mPrimitiveSelected );
    mParams->addParam( "Transformation", vector<string>(transformation,transformation+7), (int*) &mTransformationSelected );
    mParams->addParam( "Translate", &mTranslate );
    mParams->addParam( "Translate", &mTranslatexz );
    mParams->addParam( "Rotate", &mRotate );
    mParams->addParam( "RotateXZ", &mRotatexz );
    mParams->addParam( "Animate", &flagSelected );
    mParams->addParam( "Animate", &mRotate );
    
    mParams->addSeparator();
    
    {
		std::function<void(float)> setter	= std::bind( &GeometryApp::setXlim, this, std::placeholders::_1 );
		std::function<float()> getter		= std::bind( &GeometryApp::getXlim, this );
		mParams->addParam( "X-Lim", setter, getter );
	}
    
    {
		std::function<void(float)> setter	= std::bind( &GeometryApp::setYlim, this, std::placeholders::_1 );
		std::function<float()> getter		= std::bind( &GeometryApp::getYlim, this );
		mParams->addParam( "Y-Lim", setter, getter );
	}
    
    {
		std::function<void(float)> setter	= std::bind( &GeometryApp::setZlim, this, std::placeholders::_1 );
		std::function<float()> getter		= std::bind( &GeometryApp::getZlim, this );
		mParams->addParam( "Z-Lim", setter, getter );
	}
    
    mParams->addSeparator();
    
    {
		std::function<void(float)> setter	= std::bind( &GeometryApp::setRed, this, std::placeholders::_1 );
		std::function<float()> getter		= std::bind( &GeometryApp::getRed, this );
		mParams->addParam( "Red", setter, getter );
	}
    
    
    {
		std::function<void(float)> setter	= std::bind( &GeometryApp::setGreen, this, std::placeholders::_1 );
		std::function<float()> getter		= std::bind( &GeometryApp::getGreen, this );
		mParams->addParam( "Green", setter, getter );
	}
    
    {
		std::function<void(float)> setter	= std::bind( &GeometryApp::setBlue, this, std::placeholders::_1 );
		std::function<float()> getter		= std::bind( &GeometryApp::getBlue, this );
		mParams->addParam( "Blue", setter, getter );
	}
    
    mParams->addSeparator();
    
	mParams->addParam( "Quality", vector<string>(qualities,qualities+3), (int*) &mQualitySelected );
//	mParams->addParam( "Viewing Mode", vector<string>(viewmodes,viewmodes+2), (int*) &mViewMode );

	
	{
		std::function<void(int)> setter	= std::bind( &GeometryApp::setSubdivision, this, std::placeholders::_1 );
		std::function<int()> getter		= std::bind( &GeometryApp::getSubdivision, this );
		mParams->addParam( "Subdivision", setter, getter );
	}

	mParams->addSeparator();

	mParams->addParam( "Show Grid", &mShowGrid );
	mParams->addParam( "Show Normals", &mShowNormals );
	{
		std::function<void(bool)> setter	= std::bind( &GeometryApp::enableColors, this, std::placeholders::_1 );
		std::function<bool()> getter		= std::bind( &GeometryApp::isColorsEnabled, this );
		mParams->addParam( "Show Colors", setter, getter );
	}
#endif
}

void GeometryApp::createGrid()
{
	mGrid = gl::VertBatch::create( GL_LINES );
	mGrid->begin( GL_LINES );
	mGrid->color( Color(0.25f, 0.25f, 0.25f) ); mGrid->vertex( -10.0f, 0.0f, 0.0f );
	mGrid->color( Color(0.25f, 0.25f, 0.25f) ); mGrid->vertex( 0.0f, 0.0f, 0.0f );
	mGrid->color( Color(1, 0, 0) ); mGrid->vertex( 0.0f, 0.0f, 0.0f );
	mGrid->color( Color(1, 0, 0) ); mGrid->vertex( 20.0f, 0.0f, 0.0f );
	mGrid->color( Color(0, 1, 0) ); mGrid->vertex( 0.0f, 0.0f, 0.0f );
	mGrid->color( Color(0, 1, 0) ); mGrid->vertex( 0.0f, 20.0f, 0.0f );
	mGrid->color( Color(0.25f, 0.25f, 0.25f) ); mGrid->vertex( 0.0f, 0.0f, -10.0f );
	mGrid->color( Color(0.25f, 0.25f, 0.25f) ); mGrid->vertex( 0.0f, 0.0f, 0.0f );
	mGrid->color( Color(0, 0, 1) ); mGrid->vertex( 0.0f, 0.0f, 0.0f );
	mGrid->color( Color(0, 0, 1) ); mGrid->vertex( 0.0f, 0.0f, 20.0f );
	for( int i = -10; i <= 10; ++i ) {
		if( i == 0 )
			continue;

		mGrid->color( Color(0.25f, 0.25f, 0.25f) );
		mGrid->color( Color(0.25f, 0.25f, 0.25f) );
		mGrid->color( Color(0.25f, 0.25f, 0.25f) );
		mGrid->color( Color(0.25f, 0.25f, 0.25f) );
		
		mGrid->vertex( float(i), 0.0f, -10.0f );
		mGrid->vertex( float(i), 0.0f, +10.0f );
		mGrid->vertex( -10.0f, 0.0f, float(i) );
		mGrid->vertex( +10.0f, 0.0f, float(i) );
	}
	mGrid->end();
}

void GeometryApp::createPrimitive(void)
{
	geom::SourceRef primitive;
	std::string     name;

	switch( mPrimitiveCurrent ) {
	default:
		mPrimitiveSelected = CAPSULE;
	case CAPSULE:
		switch(mQualityCurrent) {
			case DEFAULT: primitive = geom::SourceRef( new geom::Capsule( geom::Capsule() ) ); break;
			case LOW: primitive = geom::SourceRef( new geom::Capsule( geom::Capsule().subdivisionsAxis( 6 ).subdivisionsHeight( 1 ) ) ); break;
			case HIGH: primitive = geom::SourceRef( new geom::Capsule( geom::Capsule().subdivisionsAxis( 60 ).subdivisionsHeight( 20 ) ) ); break;
		}
		break;
	case CONE:
		switch(mQualityCurrent) {
			case DEFAULT: primitive = geom::SourceRef( new geom::Cone() ); break;
			case LOW: primitive = geom::SourceRef( new geom::Cone( geom::Cone().subdivisionsAxis( 6 ).subdivisionsHeight( 1 ) ) ); break;
			case HIGH: primitive = geom::SourceRef( new geom::Cone( geom::Cone().subdivisionsAxis( 60 ).subdivisionsHeight( 60 ) ) ); break;
		}
		break;
	case CUBE:
		primitive = geom::SourceRef( new geom::Cube( geom::Cube() ) );
		break;
	case CYLINDER:
		switch(mQualityCurrent) {
			case DEFAULT: primitive = geom::SourceRef( new geom::Cylinder( geom::Cylinder() ) ); break;
			case LOW: primitive = geom::SourceRef( new geom::Cylinder( geom::Cylinder().subdivisionsAxis( 6 ) ) ); break;
			case HIGH: primitive = geom::SourceRef( new geom::Cylinder( geom::Cylinder().subdivisionsAxis( 60 ).subdivisionsHeight( 20 ) ) ); break;
		}
		break;
	case HELIX:
		switch(mQualityCurrent) {
			case DEFAULT: primitive = geom::SourceRef( new geom::Helix( geom::Helix() ) ); break;
			case LOW: primitive = geom::SourceRef( new geom::Helix( geom::Helix().subdivisionsHeight( 12 ).subdivisionsHeight( 6 ) ) ); break;
			case HIGH: primitive = geom::SourceRef( new geom::Helix( geom::Helix().subdivisionsHeight( 60 ).subdivisionsHeight( 60 ) ) ); break;
		}
		break;
	case ICOSAHEDRON:
		primitive = geom::SourceRef( new geom::Icosahedron( geom::Icosahedron() ) );
		break;
	case ICOSPHERE:
		switch(mQualityCurrent) {
			case DEFAULT: primitive = geom::SourceRef( new geom::Icosphere( geom::Icosphere() ) ); break;
			case LOW: primitive = geom::SourceRef( new geom::Icosphere( geom::Icosphere().subdivisions( 1 ) ) ); break;
			case HIGH: primitive = geom::SourceRef( new geom::Icosphere( geom::Icosphere().subdivisions( 5 ) ) ); break;
		}
		break;
	case SPHERE:
		switch(mQualityCurrent) {
			case DEFAULT: primitive = geom::SourceRef( new geom::Sphere( geom::Sphere() ) ); break;
			case LOW: primitive = geom::SourceRef( new geom::Sphere( geom::Sphere().subdivisions( 6 ) ) ); break;
			case HIGH: primitive = geom::SourceRef( new geom::Sphere( geom::Sphere().subdivisions( 60 ) ) ); break;
		}
		break;
	case TEAPOT:
		switch(mQualityCurrent) {
			case DEFAULT: primitive = geom::SourceRef( new geom::Teapot( geom::Teapot() ) ); break;
			case LOW: primitive = geom::SourceRef( new geom::Teapot( geom::Teapot().subdivisions( 2 ) ) ); break;
			case HIGH: primitive = geom::SourceRef( new geom::Teapot( geom::Teapot().subdivisions( 12 ) ) ); break;
		}
		break;
	case TORUS:
		switch(mQualityCurrent) {
			case DEFAULT: primitive = geom::SourceRef( new geom::Torus( geom::Torus() ) ); break;
			case LOW: primitive = geom::SourceRef( new geom::Torus( geom::Torus().subdivisionsAxis( 12 ).subdivisionsHeight( 6 ) ) ); break;
			case HIGH: primitive = geom::SourceRef( new geom::Torus( geom::Torus().subdivisionsAxis( 60 ).subdivisionsHeight( 60 ) ) ); break;
		}
		break;
	case PLANE:
			ivec2 numSegments;
			switch( mQualityCurrent ) {
				case DEFAULT: numSegments = ivec2( 10, 10 ); break;
				case LOW: numSegments = ivec2( 2, 2 ); break;
				case HIGH: numSegments = ivec2( 100, 100 ); break;
			}

			auto plane = geom::Plane().subdivisions( numSegments );

//			plane.normal( vec3( 0, 0, 1 ) ); // change the normal angle of the plane
//			plane.axes( vec3( 0.70710678118, -0.70710678118, 0 ), vec3( 0.70710678118, 0.70710678118, 0 ) ); // dictate plane u/v axes directly
//			plane.subdivisions( ivec2( 3, 10 ) ).size( vec2( 0.5f, 2.0f ) ).origin( vec3( 0, 1.0f, 0 ) ).normal( vec3( 0, 0, 1 ) ); // change the size and origin so that it is tall and thin, above the y axis.

			primitive = geom::SourceRef( new geom::Plane( plane ) );

			break;
	}

	if( mShowColors )
		primitive->enable( geom::Attrib::COLOR );
	
	TriMesh mesh( *primitive );
	mCameraCOI = mesh.calcBoundingBox().getCenter();
    //mCameraCOI += mCameraCOI + vec3(0.0,0.0,5.0);
	mRecenterCamera = true;

	if(mSubdivision > 1)
		mesh.subdivide(mSubdivision);

	
    switch (mTransformation) {
        case PLA:
            mPrimitive = gl::Batch::create( mesh, mPlaneShader );
            break;
        case TWIST:
            mPrimitive = gl::Batch::create( mesh, mTwistShader );
            break;
        case SQUASH:
            mPrimitive = gl::Batch::create( mesh, mSquashShader );
            break;
        case SQUASH2:
            mPrimitive = gl::Batch::create( mesh, mSquashShader2 );
            break;
        case SPH:
            mPrimitive = gl::Batch::create( mesh, mSphereShader );
            break;
        case CUSTOM23:
            mPrimitive = gl::Batch::create( mesh, mCustomShader23 );
            break;
        case CUSTOM123:
            mPrimitive = gl::Batch::create( mesh, mCustomShader123 );
            break;
        default:
            mPrimitive = gl::Batch::create( mesh, mPlaneShader );
            break;
    }
    
    
	mPrimitiveWireframe = gl::Batch::create( mesh, mWireframeShader );
	mNormals = gl::Batch::create( DebugMesh( mesh, Color(1,1,0) ), gl::context()->getStockShader( gl::ShaderDef().color() ) );
    mNormals_to_plane = gl::Batch::create( DebugMesh( mesh, Color(1,1,0) ), gl::context()->getStockShader( gl::ShaderDef().color() ) );

	getWindow()->setTitle( "Transform");
}



void GeometryApp::createCustomShader123(void){
    try {
		mCustomShader123 = gl::GlslProg::create( gl::GlslProg::Format()
                                               .vertex(
                                                       "#version 150\n"
                                                       "\n"
                                                       "uniform mat4	ciModelViewProjection;\n"
                                                       "uniform mat4	ciModelView;\n"
                                                       "uniform mat3	ciNormalMatrix;\n"
                                                       "uniform float	mixAmount;\n"
                                                       "uniform vec3 worldUp;\n"
                                                       "uniform float ciElapsedSeconds;\n"
                                                       "uniform float angle_deg_max;\n"
                                                       "uniform float height;\n"
                                                       "uniform vec3 centerPoint;\n"
                                                       "const float PI = 3.1415926535897932384626433832795;\n"
                                                       "\n"
                                                       "in vec4		ciPosition;\n"
                                                       "in vec3		ciNormal;\n"
                                                       "in vec4		ciColor;\n"
                                                       "\n"
                                                       
                                                       "uniform float xlim;\n"
                                                       "uniform float ylim;\n"
                                                       "uniform float zlim;\n"
                                                       
                                                       "in vec2		ciTexCoord0;\n"
                                                       "out vec2 vUv;\n"
                                                       "uniform bool flag;\n"
                                                       "\n"
                                                       "out VertexData {\n"
                                                       "	vec4 position;\n"
                                                       "	vec3 normal;\n"
                                                       "	vec4 color;\n"
                                                       "} vVertexOut;\n"
                                                       "\n"
                                                       
                                                       "vec4 stretch(vec4 pos,bool stretch){\n"
                                                       "float disty = centerPoint.y - pos.y;\n"
                                                       "float distx = centerPoint.x - pos.x;\n"
                                                       "float distz = centerPoint.z - pos.z;\n"
                                                       "float dist = abs(distance(centerPoint,vec3(pos)));\n"
                                                       "  vec4 new_pos;\n"
                                                       "  new_pos.y = ylim * dist/2 * pos.y;\n"
                                                       "  //new_pos.y = sin( -PI * sqrt(length(pos.xz)));\n"
                                                       "  new_pos.x = xlim * dist/2 * pos.x;\n"
                                                       "  new_pos.z = zlim * dist * pos.z;\n"
                                                       "\n"
                                                       "  //new_pos.y = pos.z;\n"
                                                       "  //new_pos.x = dist * pos.y;\n"
                                                       "  //new_pos.z = dist * pos.x;\n"
                                                       "\n"
                                                       "  //new_pos.z = pos.z;\n"
                                                       "  new_pos.w = pos.w;\n"
                                                       "  return new_pos;\n"
                                                       "}\n"
                                                       
                                                       "\n"
                                                       "   vec4 DoTwist(vec4 pos, float t){\n"
                                                       "        float st = sin(t);\n"
                                                       "        float ct = cos(t);\n"
                                                       "        vec4 new_pos;\n"
                                                       "        new_pos.y = pos.y;\n"
                                                       "        new_pos.z = pos.x * st + pos.z * ct;\n"
                                                       "        new_pos.x = pos.x * ct - pos.z * st;\n"
                                                       "        new_pos.w = pos.w;\n"
                                                       "\n"
                                                       "        return(new_pos);\n"
                                                       "    }\n"
                                                       "\n"
                                                       "void main(void) {\n"
                                                       "float angle_deg = angle_deg_max * sin(ciElapsedSeconds);\n"
                                                       "float angle_rad = angle_deg * 3.14159 / 180.0;\n"
                                                       "float ang = (height*0.5 + ciPosition.y)/height * angle_rad;\n"
                                                       "vec4 twistedPosition = DoTwist(ciPosition,ang);\n"
                                                       "vec4 twistedNormal = DoTwist(vec4(ciNormal,1.0),ang);\n"
                                                       "float mxAm = 0.1 * (1.0 + sin(ciElapsedSeconds));\n"
                                                       "float mxAm2 = 0.5 * (1.0 + sin(ciElapsedSeconds));\n"
                                                       
                                                       "vec3 goalPosition = 5.0 * vec3(-ciTexCoord0.x,ciTexCoord0.y,-ciTexCoord0.x) ;\n"
                                                       "vec4 newPosition_tt = mix(ciPosition,vec4(goalPosition,1.0),mxAm2);\n"
                                                       "vec3 wU_tt = cross(worldUp,vec3(newPosition_tt));\n"
                                                       "vec3 newNormal_tt = mix(ciNormal,wU_tt,mxAm2);\n"
                                                       "vUv = ciTexCoord0;\n"
                                                       
                                                       "vec4 stretch_new_pos = stretch(twistedPosition,true);\n"
                                                       "vec4 stretch_new_normal = stretch(twistedNormal,true);\n"
                                                       
                                                       "//vec4 newPosition = mix(ciPosition,vec4(goalPosition,1.0),mxAm);\n"
                                                       "//vec3 wU = cross(worldUp,vec3(newPosition));\n"
                                                       "//vec3 newNormal = mix(ciNormal,wU,mxAm);\n"
                                                       "\n"
                                                       "vec4 newPosition_t = mix(newPosition_tt,stretch_new_pos,mxAm);\n"
                                                       "vec3 wU_t = cross(worldUp,vec3(stretch_new_pos));\n"
                                                       "vec3 newNormal_t = mix(vec3(newNormal_tt),vec3(stretch_new_normal),mxAm);\n"
                                                       
                                                       "vec4 newPosition = mix(newPosition_t,twistedPosition,mxAm);\n"
                                                       "vec3 wU = cross(worldUp,vec3(twistedPosition));\n"
                                                       "vec3 newNormal = mix(newNormal_t,vec3(twistedNormal),mxAm);\n"
                                                       
                                                       "//vec4 newPosition = mix(twistedPosition,stretch_new_pos,mxAm);\n"
                                                       "//vec3 wU = cross(worldUp,vec3(stretch_new_pos));\n"
                                                       "//vec3 newNormal = mix(vec3(twistedNormal),vec3(stretch_new_normal),mxAm);\n"
                                                       "\n"
                                                       "	//vVertexOut.position = ciModelView * ciPosition;\n"
                                                       "	vVertexOut.position = ciModelView * newPosition;\n"
                                                       "	vVertexOut.normal = ciNormalMatrix * newNormal;\n"
                                                       "	//vVertexOut.position = ciModelView * twistedPosition;\n"
                                                       "	//vVertexOut.normal = ciNormalMatrix * vec3(twistedNormal);\n"
                                                       "	vVertexOut.color = ciColor;\n"
                                                       "	//gl_Position = ciModelViewProjection * ciPosition;\n"
                                                       "	gl_Position = ciModelViewProjection * newPosition;\n"
                                                       "	//gl_Position = ciModelViewProjection * twistedPosition;\n"
                                                       "}\n"
                                                       )
                                               .fragment(
                                                         "#version 150\n"
                                                         "\n"
                                                         "in VertexData	{\n"
                                                         "	vec4 position;\n"
                                                         "	vec3 normal;\n"
                                                         "	vec4 color;\n"
                                                         "} vVertexIn;\n"
                                                         "\n"
                                                         "out vec4 oColor;\n"
                                                         "in vec2 vUv;\n"
                                                         "uniform sampler2D baseTexture;\n"
                                                         "\n"
                                                         "void main(void) {\n"
                                                         "	// set diffuse and specular colors\n"
                                                         "	vec3 cDiffuse = vVertexIn.color.rgb;\n"
                                                         "	vec3 cSpecular = vec3(0.3, 0.3, 0.3);\n"
                                                         "\n"
                                                         "	// light properties in view space\n"
                                                         "	vec3 vLightPosition = vec3(0.0, 0.0, 0.0);\n"
                                                         "\n"
                                                         "	// lighting calculations\n"
                                                         "	vec3 vVertex = vVertexIn.position.xyz;\n"
                                                         "	vec3 vNormal = normalize( vVertexIn.normal );\n"
                                                         "	vec3 vToLight = normalize( vLightPosition - vVertex );\n"
                                                         "	vec3 vToEye = normalize( -vVertex );\n"
                                                         "	vec3 vReflect = normalize( -reflect(vToLight, vNormal) );\n"
                                                         "\n"
                                                         "	// diffuse coefficient\n"
                                                         "	vec3 diffuse = max( dot( vNormal, vToLight ), 0.0 ) * cDiffuse;\n"
                                                         "\n"
                                                         "	// specular coefficient with energy conservation\n"
                                                         "	const float shininess = 20.0;\n"
                                                         "	const float coeff = (2.0 + shininess) / (2.0 * 3.14159265);\n"
                                                         "	vec3 specular = pow( max( dot( vReflect, vToEye ), 0.0 ), shininess ) * coeff * cSpecular;\n"
                                                         "\n"
                                                         "	// to conserve energy, diffuse and specular colors should not exceed one\n"
                                                         "	float maxDiffuse = max(diffuse.r, max(diffuse.g, diffuse.b));\n"
                                                         "	float maxSpecular = max(specular.r, max(specular.g, specular.b));\n"
                                                         "	float fConserve = 1.0 / max(1.0, maxDiffuse + maxSpecular);\n"
                                                         "\n"
                                                         "	// final color\n"
                                                         "	oColor.rgb = (diffuse + specular) * fConserve;\n"
                                                         "	//oColor.rgb = vec3(texture(baseTexture,vUv),0.0);\n"
                                                         "	oColor.a = 1.0;\n"
                                                         "}\n"
                                                         )
                                               );
	}
	catch( const std::exception& e ) {
		console() << e.what() << std::endl;
	}
}




void GeometryApp::createCustomShader23(void){
    try {
		mCustomShader23 = gl::GlslProg::create( gl::GlslProg::Format()
                                            .vertex(
                                                    "#version 150\n"
                                                    "\n"
                                                    "uniform mat4	ciModelViewProjection;\n"
                                                    "uniform mat4	ciModelView;\n"
                                                    "uniform mat3	ciNormalMatrix;\n"
                                                    "uniform float	mixAmount;\n"
                                                    "uniform vec3 worldUp;\n"
                                                    "uniform float ciElapsedSeconds;\n"
                                                    "uniform float angle_deg_max;\n"
                                                    "uniform float height;\n"
                                                    "uniform vec3 centerPoint;\n"
                                                    "const float PI = 3.1415926535897932384626433832795;\n"
                                                    "\n"
                                                    "in vec4		ciPosition;\n"
                                                    "in vec3		ciNormal;\n"
                                                    "in vec4		ciColor;\n"
                                                    "\n"
                                                    
                                                    "uniform float xlim;\n"
                                                    "uniform float ylim;\n"
                                                    "uniform float zlim;\n"
                                                    
                                                    "in vec2		ciTexCoord0;\n"
                                                    "out vec2 vUv;\n"
                                                    "uniform bool flag;\n"
                                                    "\n"
                                                    "out VertexData {\n"
                                                    "	vec4 position;\n"
                                                    "	vec3 normal;\n"
                                                    "	vec4 color;\n"
                                                    "} vVertexOut;\n"
                                                    "\n"
                                                    
                                                    "vec4 stretch(vec4 pos,bool stretch){\n"
                                                    "float disty = centerPoint.y - pos.y;\n"
                                                    "float distx = centerPoint.x - pos.x;\n"
                                                    "float distz = centerPoint.z - pos.z;\n"
                                                    "float dist = abs(distance(centerPoint,vec3(pos)));\n"
                                                    "  vec4 new_pos;\n"
                                                    "  new_pos.y = ylim * dist/2 * pos.y;\n"
                                                    "  //new_pos.y = sin( -PI * sqrt(length(pos.xz)));\n"
                                                    "  new_pos.x = xlim * dist/2 * pos.x;\n"
                                                    "  new_pos.z = zlim * dist * pos.z;\n"
                                                    "\n"
                                                    "  //new_pos.y = pos.z;\n"
                                                    "  //new_pos.x = dist * pos.y;\n"
                                                    "  //new_pos.z = dist * pos.x;\n"
                                                    "\n"
                                                    "  //new_pos.z = pos.z;\n"
                                                    "  new_pos.w = pos.w;\n"
                                                    "  return new_pos;\n"
                                                    "}\n"
                                                    
                                                    "\n"
                                                    "   vec4 DoTwist(vec4 pos, float t){\n"
                                                    "        float st = sin(t);\n"
                                                    "        float ct = cos(t);\n"
                                                    "        vec4 new_pos;\n"
                                                    "        new_pos.y = pos.y;\n"
                                                    "        new_pos.z = pos.x * st + pos.z * ct;\n"
                                                    "        new_pos.x = pos.x * ct - pos.z * st;\n"
                                                    "        new_pos.w = pos.w;\n"
                                                    "\n"
                                                    "        return(new_pos);\n"
                                                    "    }\n"
                                                    "\n"
                                                    "void main(void) {\n"
                                                    "float angle_deg = angle_deg_max * sin(ciElapsedSeconds);\n"
                                                    "float angle_rad = angle_deg * 3.14159 / 180.0;\n"
                                                    "float ang = (height*0.5 + ciPosition.y)/height * angle_rad;\n"
                                                    "vec4 twistedPosition = DoTwist(ciPosition,ang);\n"
                                                    "vec4 twistedNormal = DoTwist(vec4(ciNormal,1.0),ang);\n"
                                                    "float mxAm = 0.1 * (1.0 + sin(ciElapsedSeconds));\n"
                                                    "float mxAm2 = 0.5 * (1.0 + sin(ciElapsedSeconds));\n"
                                                    "vUv = ciTexCoord0;\n"
                                                    
                                                    "vec4 stretch_new_pos = stretch(twistedPosition,true);\n"
                                                    "vec4 stretch_new_normal = stretch(twistedNormal,true);\n"
                                                    
                                                    "//vec4 newPosition = mix(ciPosition,vec4(goalPosition,1.0),mxAm);\n"
                                                    "//vec3 wU = cross(worldUp,vec3(newPosition));\n"
                                                    "//vec3 newNormal = mix(ciNormal,wU,mxAm);\n"
                                                    "\n"
                                                    "vec4 newPosition_t = mix(ciPosition,stretch_new_pos,mxAm);\n"
                                                    "vec3 wU_t = cross(worldUp,vec3(stretch_new_pos));\n"
                                                    "vec3 newNormal_t = mix(vec3(ciNormal),vec3(stretch_new_normal),mxAm);\n"
                                                    
                                                    "vec4 newPosition = mix(newPosition_t,twistedPosition,mxAm);\n"
                                                    "vec3 wU = cross(worldUp,vec3(twistedPosition));\n"
                                                    "vec3 newNormal = mix(newNormal_t,vec3(twistedNormal),mxAm);\n"
                                                    
                                                    "//vec4 newPosition = mix(twistedPosition,stretch_new_pos,mxAm);\n"
                                                    "//vec3 wU = cross(worldUp,vec3(stretch_new_pos));\n"
                                                    "//vec3 newNormal = mix(vec3(twistedNormal),vec3(stretch_new_normal),mxAm);\n"
                                                    "\n"
                                                    "	//vVertexOut.position = ciModelView * ciPosition;\n"
                                                    "	vVertexOut.position = ciModelView * newPosition;\n"
                                                    "	vVertexOut.normal = ciNormalMatrix * newNormal;\n"
                                                    "	//vVertexOut.position = ciModelView * twistedPosition;\n"
                                                    "	//vVertexOut.normal = ciNormalMatrix * vec3(twistedNormal);\n"
                                                    "	vVertexOut.color = ciColor;\n"
                                                    "	//gl_Position = ciModelViewProjection * ciPosition;\n"
                                                    "	gl_Position = ciModelViewProjection * newPosition;\n"
                                                    "	//gl_Position = ciModelViewProjection * twistedPosition;\n"
                                                    "}\n"
                                                    )
                                            .fragment(
                                                      "#version 150\n"
                                                      "\n"
                                                      "in VertexData	{\n"
                                                      "	vec4 position;\n"
                                                      "	vec3 normal;\n"
                                                      "	vec4 color;\n"
                                                      "} vVertexIn;\n"
                                                      "\n"
                                                      "out vec4 oColor;\n"
                                                      "in vec2 vUv;\n"
                                                      "uniform sampler2D baseTexture;\n"
                                                      "\n"
                                                      "void main(void) {\n"
                                                      "	// set diffuse and specular colors\n"
                                                      "	vec3 cDiffuse = vVertexIn.color.rgb;\n"
                                                      "	vec3 cSpecular = vec3(0.3, 0.3, 0.3);\n"
                                                      "\n"
                                                      "	// light properties in view space\n"
                                                      "	vec3 vLightPosition = vec3(0.0, 0.0, 0.0);\n"
                                                      "\n"
                                                      "	// lighting calculations\n"
                                                      "	vec3 vVertex = vVertexIn.position.xyz;\n"
                                                      "	vec3 vNormal = normalize( vVertexIn.normal );\n"
                                                      "	vec3 vToLight = normalize( vLightPosition - vVertex );\n"
                                                      "	vec3 vToEye = normalize( -vVertex );\n"
                                                      "	vec3 vReflect = normalize( -reflect(vToLight, vNormal) );\n"
                                                      "\n"
                                                      "	// diffuse coefficient\n"
                                                      "	vec3 diffuse = max( dot( vNormal, vToLight ), 0.0 ) * cDiffuse;\n"
                                                      "\n"
                                                      "	// specular coefficient with energy conservation\n"
                                                      "	const float shininess = 20.0;\n"
                                                      "	const float coeff = (2.0 + shininess) / (2.0 * 3.14159265);\n"
                                                      "	vec3 specular = pow( max( dot( vReflect, vToEye ), 0.0 ), shininess ) * coeff * cSpecular;\n"
                                                      "\n"
                                                      "	// to conserve energy, diffuse and specular colors should not exceed one\n"
                                                      "	float maxDiffuse = max(diffuse.r, max(diffuse.g, diffuse.b));\n"
                                                      "	float maxSpecular = max(specular.r, max(specular.g, specular.b));\n"
                                                      "	float fConserve = 1.0 / max(1.0, maxDiffuse + maxSpecular);\n"
                                                      "\n"
                                                      "	// final color\n"
                                                      "	oColor.rgb = (diffuse + specular) * fConserve;\n"
                                                      "	//oColor.rgb = vec3(texture(baseTexture,vUv),0.0);\n"
                                                      "	oColor.a = 1.0;\n"
                                                      "}\n"
                                                      )
                                            );
	}
	catch( const std::exception& e ) {
		console() << e.what() << std::endl;
	}
}

void GeometryApp::createTwistShader(void){
	try {
		mTwistShader = gl::GlslProg::create( gl::GlslProg::Format()
                                            .vertex(
                                                    "#version 150\n"
                                                    "\n"
                                                    "uniform mat4	ciModelViewProjection;\n"
                                                    "uniform mat4	ciModelView;\n"
                                                    "uniform mat3	ciNormalMatrix;\n"
                                                    "uniform float	mixAmount;\n"
                                                    "uniform vec3 worldUp;\n"
                                                    "uniform float ciElapsedSeconds;\n"
                                                    "uniform float angle_deg_max;\n"
                                                    "uniform float height;\n"
                                                    "uniform vec3 centerPoint;\n"
                                                    "const float PI = 3.1415926535897932384626433832795;\n"
                                                    "\n"
                                                    "in vec4		ciPosition;\n"
                                                    "in vec3		ciNormal;\n"
                                                    "in vec4		ciColor;\n"
                                                    "\n"
                                                    "in vec2		ciTexCoord0;\n"
                                                    "out vec2 vUv;\n"
                                                    "uniform bool flag;\n"
                                                    "\n"
                                                    "out VertexData {\n"
                                                    "	vec4 position;\n"
                                                    "	vec3 normal;\n"
                                                    "	vec4 color;\n"
                                                    "} vVertexOut;\n"
                                                    "\n"
                                                    "\n"
                                                    "   vec4 DoTwist(vec4 pos, float t){\n"
                                                    "        float st = sin(t);\n"
                                                    "        float ct = cos(t);\n"
                                                    "        vec4 new_pos;\n"
                                                    "        new_pos.y = pos.y;\n"
                                                    "        new_pos.z = pos.x * st + pos.z * ct;\n"
                                                    "        new_pos.x = pos.x * ct - pos.z * st;\n"
                                                    "        new_pos.w = pos.w;\n"
                                                    "\n"
                                                    "        return(new_pos);\n"
                                                    "    }\n"
                                                    "\n"
                                                    "void main(void) {\n"
                                                    "float angle_deg = angle_deg_max * sin(ciElapsedSeconds);\n"
                                                    "float angle_rad = angle_deg * 3.14159 / 180.0;\n"
                                                    "float ang = (height*0.5 + ciPosition.y)/height * angle_rad;\n"
                                                    "vec4 twistedPosition = DoTwist(ciPosition,ang);\n"
                                                    "vec4 twistedNormal = DoTwist(vec4(ciNormal,1.0),ang);\n"
                                                    "float mxAm = 0.2 * (1.0 + sin(ciElapsedSeconds));\n"
                                                    "vUv = ciTexCoord0;\n"
                                                    
                                                    
                                                    "//vec4 newPosition = mix(ciPosition,vec4(goalPosition,1.0),mxAm);\n"
                                                    "//vec3 wU = cross(worldUp,vec3(newPosition));\n"
                                                    "//vec3 newNormal = mix(ciNormal,wU,mxAm);\n"
                                                    "\n"
                                                    "vec4 newPosition = mix(ciPosition,twistedPosition,mxAm);\n"
                                                    "vec3 wU = cross(worldUp,vec3(twistedPosition));\n"
                                                    "vec3 newNormal = mix(ciNormal,vec3(twistedNormal),mxAm);\n"
                                                    "\n"
                                                    "	//vVertexOut.position = ciModelView * ciPosition;\n"
                                                    "	vVertexOut.position = ciModelView * newPosition;\n"
                                                    "	vVertexOut.normal = ciNormalMatrix * newNormal;\n"
                                                    "	//vVertexOut.position = ciModelView * twistedPosition;\n"
                                                    "	//vVertexOut.normal = ciNormalMatrix * vec3(twistedNormal);\n"
                                                    "	vVertexOut.color = ciColor;\n"
                                                    "	//gl_Position = ciModelViewProjection * ciPosition;\n"
                                                    "	gl_Position = ciModelViewProjection * newPosition;\n"
                                                    "	//gl_Position = ciModelViewProjection * twistedPosition;\n"
                                                    "}\n"
                                                    )
                                            .fragment(
                                                      "#version 150\n"
                                                      "\n"
                                                      "in VertexData	{\n"
                                                      "	vec4 position;\n"
                                                      "	vec3 normal;\n"
                                                      "	vec4 color;\n"
                                                      "} vVertexIn;\n"
                                                      "\n"
                                                      "out vec4 oColor;\n"
                                                      "in vec2 vUv;\n"
                                                      "uniform sampler2D baseTexture;\n"
                                                      "\n"
                                                      "void main(void) {\n"
                                                      "	// set diffuse and specular colors\n"
                                                      "	vec3 cDiffuse = vVertexIn.color.rgb;\n"
                                                      "	vec3 cSpecular = vec3(0.3, 0.3, 0.3);\n"
                                                      "\n"
                                                      "	// light properties in view space\n"
                                                      "	vec3 vLightPosition = vec3(0.0, 0.0, 0.0);\n"
                                                      "\n"
                                                      "	// lighting calculations\n"
                                                      "	vec3 vVertex = vVertexIn.position.xyz;\n"
                                                      "	vec3 vNormal = normalize( vVertexIn.normal );\n"
                                                      "	vec3 vToLight = normalize( vLightPosition - vVertex );\n"
                                                      "	vec3 vToEye = normalize( -vVertex );\n"
                                                      "	vec3 vReflect = normalize( -reflect(vToLight, vNormal) );\n"
                                                      "\n"
                                                      "	// diffuse coefficient\n"
                                                      "	vec3 diffuse = max( dot( vNormal, vToLight ), 0.0 ) * cDiffuse;\n"
                                                      "\n"
                                                      "	// specular coefficient with energy conservation\n"
                                                      "	const float shininess = 20.0;\n"
                                                      "	const float coeff = (2.0 + shininess) / (2.0 * 3.14159265);\n"
                                                      "	vec3 specular = pow( max( dot( vReflect, vToEye ), 0.0 ), shininess ) * coeff * cSpecular;\n"
                                                      "\n"
                                                      "	// to conserve energy, diffuse and specular colors should not exceed one\n"
                                                      "	float maxDiffuse = max(diffuse.r, max(diffuse.g, diffuse.b));\n"
                                                      "	float maxSpecular = max(specular.r, max(specular.g, specular.b));\n"
                                                      "	float fConserve = 1.0 / max(1.0, maxDiffuse + maxSpecular);\n"
                                                      "\n"
                                                      "	// final color\n"
                                                      "	oColor.rgb = (diffuse + specular) * fConserve;\n"
                                                      "	//oColor.rgb = vec3(texture(baseTexture,vUv),0.0);\n"
                                                      "	oColor.a = 1.0;\n"
                                                      "}\n"
                                                      )
                                            );
	}
	catch( const std::exception& e ) {
		console() << e.what() << std::endl;
	}
}

void GeometryApp::createSquashShader(void){
    try {
		mSquashShader = gl::GlslProg::create( gl::GlslProg::Format()
                                            .vertex(
                                                    "#version 150\n"
                                                    "\n"
                                                    "uniform mat4	ciModelViewProjection;\n"
                                                    "uniform mat4	ciModelView;\n"
                                                    "uniform mat3	ciNormalMatrix;\n"
                                                    "uniform float	mixAmount;\n"
                                                    "uniform vec3 worldUp;\n"
                                                    "uniform float ciElapsedSeconds;\n"
                                                    "uniform float angle_deg_max;\n"
                                                    "uniform float height;\n"
                                                    "uniform vec3 centerPoint;\n"
                                                    
                                                    "uniform float xlim;\n"
                                                    "uniform float ylim;\n"
                                                    "uniform float zlim;\n"
                                                    
                                                    "const float PI = 3.1415926535897932384626433832795;\n"
                                                    "\n"
                                                    "in vec4		ciPosition;\n"
                                                    "in vec3		ciNormal;\n"
                                                    "in vec4		ciColor;\n"
                                                    "\n"
                                                    "in vec2		ciTexCoord0;\n"
                                                    "out vec2 vUv;\n"
                                                    "uniform bool flag;\n"
                                                    "\n"
                                                    "out VertexData {\n"
                                                    "	vec4 position;\n"
                                                    "	vec3 normal;\n"
                                                    "	vec4 color;\n"
                                                    "} vVertexOut;\n"
                                                    "\n"
                                                    "\n"

                                                    "vec4 stretch(vec4 pos,bool stretch){\n"
                                                    "float disty = centerPoint.y - pos.y;\n"
                                                    "float distx = centerPoint.x - pos.x;\n"
                                                    "float distz = centerPoint.z - pos.z;\n"
                                                    "float dist = abs(distance(centerPoint,vec3(pos)));\n"
                                                    "  vec4 new_pos;\n"
                                                    "  new_pos.y = ylim * dist/2 * pos.y;\n"
                                                    "  //new_pos.y = sin( -PI * sqrt(length(pos.xz)));\n"
                                                    "  new_pos.x = xlim * dist/2 * pos.x;\n"
                                                    "  new_pos.z = zlim * dist * pos.z;\n"
                                                    "\n"
                                                    "  //new_pos.y = pos.z;\n"
                                                    "  //new_pos.x = dist * pos.y;\n"
                                                    "  //new_pos.z = dist * pos.x;\n"
                                                    "\n"
                                                    "  //new_pos.z = pos.z;\n"
                                                    "  new_pos.w = pos.w;\n"
                                                    "  return new_pos;\n"
                                                    "}\n"
                                                    "void main(void) {\n"
                                                    "float mxAm = 0.5 * (1.0 + sin(ciElapsedSeconds));\n"
                                                    "vUv = ciTexCoord0;\n"
                                                    
                                                    "vec4 stretch_new_pos = stretch(ciPosition,true);\n"
                                                    "vec4 stretch_new_normal = stretch(vec4(ciNormal,1.0),true);\n"
                                                    
                                                    "//vec4 newPosition = mix(ciPosition,vec4(goalPosition,1.0),mxAm);\n"
                                                    "//vec3 wU = cross(worldUp,vec3(newPosition));\n"
                                                    "//vec3 newNormal = mix(ciNormal,wU,mxAm);\n"
                                                    "\n"
                                                    "vec4 newPosition = mix(ciPosition,stretch_new_pos,mxAm);\n"
                                                    "vec3 wU = cross(worldUp,vec3(stretch_new_pos));\n"
                                                    "vec3 newNormal = mix(ciNormal,vec3(stretch_new_normal),mxAm);\n"
                                                    "\n"
                                                    "	//vVertexOut.position = ciModelView * ciPosition;\n"
                                                    "	vVertexOut.position = ciModelView * newPosition;\n"
                                                    "	vVertexOut.normal = ciNormalMatrix * newNormal;\n"
                                                    "	vVertexOut.color = ciColor;\n"
                                                    "	//gl_Position = ciModelViewProjection * ciPosition;\n"
                                                    "	gl_Position = ciModelViewProjection * newPosition;\n"
                                                    "}\n"
                                                    )
                                            .fragment(
                                                      "#version 150\n"
                                                      "\n"
                                                      "in VertexData	{\n"
                                                      "	vec4 position;\n"
                                                      "	vec3 normal;\n"
                                                      "	vec4 color;\n"
                                                      "} vVertexIn;\n"
                                                      "\n"
                                                      "out vec4 oColor;\n"
                                                      "in vec2 vUv;\n"
                                                      "uniform sampler2D baseTexture;\n"
                                                      "\n"
                                                      "void main(void) {\n"
                                                      "	// set diffuse and specular colors\n"
                                                      "	vec3 cDiffuse = vVertexIn.color.rgb;\n"
                                                      "	vec3 cSpecular = vec3(0.3, 0.3, 0.3);\n"
                                                      "\n"
                                                      "	// light properties in view space\n"
                                                      "	vec3 vLightPosition = vec3(0.0, 0.0, 0.0);\n"
                                                      "\n"
                                                      "	// lighting calculations\n"
                                                      "	vec3 vVertex = vVertexIn.position.xyz;\n"
                                                      "	vec3 vNormal = normalize( vVertexIn.normal );\n"
                                                      "	vec3 vToLight = normalize( vLightPosition - vVertex );\n"
                                                      "	vec3 vToEye = normalize( -vVertex );\n"
                                                      "	vec3 vReflect = normalize( -reflect(vToLight, vNormal) );\n"
                                                      "\n"
                                                      "	// diffuse coefficient\n"
                                                      "	vec3 diffuse = max( dot( vNormal, vToLight ), 0.0 ) * cDiffuse;\n"
                                                      "\n"
                                                      "	// specular coefficient with energy conservation\n"
                                                      "	const float shininess = 20.0;\n"
                                                      "	const float coeff = (2.0 + shininess) / (2.0 * 3.14159265);\n"
                                                      "	vec3 specular = pow( max( dot( vReflect, vToEye ), 0.0 ), shininess ) * coeff * cSpecular;\n"
                                                      "\n"
                                                      "	// to conserve energy, diffuse and specular colors should not exceed one\n"
                                                      "	float maxDiffuse = max(diffuse.r, max(diffuse.g, diffuse.b));\n"
                                                      "	float maxSpecular = max(specular.r, max(specular.g, specular.b));\n"
                                                      "	float fConserve = 1.0 / max(1.0, maxDiffuse + maxSpecular);\n"
                                                      "\n"
                                                      "	// final color\n"
                                                      "	oColor.rgb = (diffuse + specular) * fConserve;\n"
                                                      "	//oColor.rgb = vec3(texture(baseTexture,vUv),0.0);\n"
                                                      "	oColor.a = 1.0;\n"
                                                      "}\n"
                                                      )
                                            );
	}
	catch( const std::exception& e ) {
		console() << e.what() << std::endl;
	}
}



void GeometryApp::createSquashShader2(void){
    try {
		mSquashShader2 = gl::GlslProg::create( gl::GlslProg::Format()
                                             .vertex(
                                                     "#version 150\n"
                                                     "\n"
                                                     "uniform mat4	ciModelViewProjection;\n"
                                                     "uniform mat4	ciModelView;\n"
                                                     "uniform mat3	ciNormalMatrix;\n"
                                                     "uniform float	mixAmount;\n"
                                                     "uniform vec3 worldUp;\n"
                                                     "uniform float ciElapsedSeconds;\n"
                                                     "uniform float angle_deg_max;\n"
                                                     "uniform float height;\n"
                                                     "uniform vec3 centerPoint;\n"
                                                     
                                                     "uniform float xlim;\n"
                                                     "uniform float ylim;\n"
                                                     "uniform float zlim;\n"
                                                     
                                                     "const float PI = 3.1415926535897932384626433832795;\n"
                                                     "\n"
                                                     "in vec4		ciPosition;\n"
                                                     "in vec3		ciNormal;\n"
                                                     "in vec4		ciColor;\n"
                                                     "\n"
                                                     "in vec2		ciTexCoord0;\n"
                                                     "out vec2 vUv;\n"
                                                     "uniform bool flag;\n"
                                                     "\n"
                                                     "out VertexData {\n"
                                                     "	vec4 position;\n"
                                                     "	vec3 normal;\n"
                                                     "	vec4 color;\n"
                                                     "} vVertexOut;\n"
                                                     "\n"
                                                     "\n"
                                                     
                                                     "vec4 stretch(vec4 pos,bool stretch){\n"
                                                     "float disty = centerPoint.y - pos.y;\n"
                                                     "float distx = centerPoint.x - pos.x;\n"
                                                     "float distz = centerPoint.z - pos.z;\n"
                                                     "float dist = abs(distance(centerPoint,vec3(pos)));\n"
                                                     "  vec4 new_pos;\n"
                                                     "  new_pos.y = ylim * dist/2 * pos.y;\n"
                                                     "  //new_pos.y = sin( -PI * sqrt(length(pos.xz)));\n"
                                                     "  new_pos.x = xlim * dist/2 * pos.x;\n"
                                                     "  new_pos.z = zlim * dist * pos.z;\n"
                                                     "\n"
                                                     "  //new_pos.y = pos.z;\n"
                                                     "  //new_pos.x = dist * pos.y;\n"
                                                     "  //new_pos.z = dist * pos.x;\n"
                                                     "\n"
                                                     "  //new_pos.z = pos.z;\n"
                                                     "  new_pos.w = pos.w;\n"
                                                     "  return new_pos;\n"
                                                     "}\n"
                                                     "void main(void) {\n"
                                                     "float mxAm = 0.5 * (1.0 + sin(ciElapsedSeconds));\n"
                                                     "vUv = ciTexCoord0;\n"
                                                     
                                                     "vec4 stretch_new_pos = stretch(ciPosition,true);\n"
                                                     "vec4 stretch_new_normal = stretch(vec4(ciNormal,1.0),true);\n"
                                                     
                                                     "//vec4 newPosition = mix(ciPosition,vec4(goalPosition,1.0),mxAm);\n"
                                                     "//vec3 wU = cross(worldUp,vec3(newPosition));\n"
                                                     "//vec3 newNormal = mix(ciNormal,wU,mxAm);\n"
                                                     "\n"
                                                     "vec4 newPosition = mix(ciPosition,stretch_new_pos,mxAm);\n"
                                                     "vec4 newPosition_stretched = mix(ciPosition,stretch_new_pos,0.8);\n"
                                                     "vec3 wU = cross(worldUp,vec3(stretch_new_pos));\n"
                                                     "vec3 newNormal = mix(ciNormal,vec3(stretch_new_normal),mxAm);\n"
                                                     "vec3 newNormal_stretched = mix(ciNormal,vec3(stretch_new_normal),0.8);\n"
                                                     "\n"
                                                     "vec4 stretch_new_pos2 = stretch(newPosition_stretched,true);\n"
                                                     "vec4 stretch_new_normal2 = stretch(vec4(newNormal_stretched,1.0),true);\n"
                                                     "vec4 newPosition2 = mix(newPosition_stretched,stretch_new_pos2,mxAm);\n"
                                                     "vec3 newNormal2 = mix(newNormal_stretched,vec3(stretch_new_normal2),mxAm);\n"
                                                     "	//vVertexOut.position = ciModelView * ciPosition;\n"
                                                     "	vVertexOut.position = ciModelView * newPosition2;\n"
                                                     "	vVertexOut.normal = ciNormalMatrix * newNormal2;\n"
                                                     "	vVertexOut.color = ciColor;\n"
                                                     "	//gl_Position = ciModelViewProjection * ciPosition;\n"
                                                     "	gl_Position = ciModelViewProjection * newPosition2;\n"
                                                     "}\n"
                                                     )
                                             .fragment(
                                                       "#version 150\n"
                                                       "\n"
                                                       "in VertexData	{\n"
                                                       "	vec4 position;\n"
                                                       "	vec3 normal;\n"
                                                       "	vec4 color;\n"
                                                       "} vVertexIn;\n"
                                                       "\n"
                                                       "out vec4 oColor;\n"
                                                       "in vec2 vUv;\n"
                                                       "uniform sampler2D baseTexture;\n"
                                                       "\n"
                                                       "void main(void) {\n"
                                                       "	// set diffuse and specular colors\n"
                                                       "	vec3 cDiffuse = vVertexIn.color.rgb;\n"
                                                       "	vec3 cSpecular = vec3(0.3, 0.3, 0.3);\n"
                                                       "\n"
                                                       "	// light properties in view space\n"
                                                       "	vec3 vLightPosition = vec3(0.0, 0.0, 0.0);\n"
                                                       "\n"
                                                       "	// lighting calculations\n"
                                                       "	vec3 vVertex = vVertexIn.position.xyz;\n"
                                                       "	vec3 vNormal = normalize( vVertexIn.normal );\n"
                                                       "	vec3 vToLight = normalize( vLightPosition - vVertex );\n"
                                                       "	vec3 vToEye = normalize( -vVertex );\n"
                                                       "	vec3 vReflect = normalize( -reflect(vToLight, vNormal) );\n"
                                                       "\n"
                                                       "	// diffuse coefficient\n"
                                                       "	vec3 diffuse = max( dot( vNormal, vToLight ), 0.0 ) * cDiffuse;\n"
                                                       "\n"
                                                       "	// specular coefficient with energy conservation\n"
                                                       "	const float shininess = 20.0;\n"
                                                       "	const float coeff = (2.0 + shininess) / (2.0 * 3.14159265);\n"
                                                       "	vec3 specular = pow( max( dot( vReflect, vToEye ), 0.0 ), shininess ) * coeff * cSpecular;\n"
                                                       "\n"
                                                       "	// to conserve energy, diffuse and specular colors should not exceed one\n"
                                                       "	float maxDiffuse = max(diffuse.r, max(diffuse.g, diffuse.b));\n"
                                                       "	float maxSpecular = max(specular.r, max(specular.g, specular.b));\n"
                                                       "	float fConserve = 1.0 / max(1.0, maxDiffuse + maxSpecular);\n"
                                                       "\n"
                                                       "	// final color\n"
                                                       "	oColor.rgb = (diffuse + specular) * fConserve;\n"
                                                       "	//oColor.rgb = vec3(texture(baseTexture,vUv),0.0);\n"
                                                       "	oColor.a = 1.0;\n"
                                                       "}\n"
                                                       )
                                             );
	}
	catch( const std::exception& e ) {
		console() << e.what() << std::endl;
	}
}


void GeometryApp::createSphereShader(void){
    try {
		mSphereShader = gl::GlslProg::create( gl::GlslProg::Format()
                                             .vertex(
                                                     "#version 150\n"
                                                     "\n"
                                                     "uniform mat4	ciModelViewProjection;\n"
                                                     "uniform mat4	ciModelView;\n"
                                                     "uniform mat3	ciNormalMatrix;\n"
                                                     "uniform float	mixAmount;\n"
                                                     "uniform vec3 worldUp;\n"
                                                     "uniform float ciElapsedSeconds;\n"
                                                     "uniform float angle_deg_max;\n"
                                                     "uniform float height;\n"
                                                     "uniform vec3 centerPoint;\n"
                                                     
                                                     "uniform float xlim;\n"
                                                     "uniform float ylim;\n"
                                                     "uniform float zlim;\n"
                                                     
                                                     "const float PI = 3.1415926535897932384626433832795;\n"
                                                     "\n"
                                                     "in vec4		ciPosition;\n"
                                                     "in vec3		ciNormal;\n"
                                                     "in vec4		ciColor;\n"
                                                     "\n"
                                                     "in vec2		ciTexCoord0;\n"
                                                     "out vec2 vUv;\n"
                                                     "uniform bool flag;\n"
                                                     "\n"
                                                     "out VertexData {\n"
                                                     "	vec4 position;\n"
                                                     "	vec3 normal;\n"
                                                     "	vec4 color;\n"
                                                     "} vVertexOut;\n"
                                                     "\n"
                                                     "\n"
                                                     
                                                     "vec4 sphere(vec4 pos){\n"
                                                     "float dx = pos.x * sqrt(1.0 - (pos.y*pos.y/2.0) - (pos.z*pos.z/2.0) + (pos.y*pos.y*pos.z*pos.z / 3.0) );\n"
                                                     "float dy = pos.y * sqrt(1.0 - (pos.z*pos.z/2.0) - (pos.x*pos.x/2.0) + (pos.z*pos.z*pos.x*pos.x / 3.0) );\n"
                                                     "float dz = pos.z * sqrt(1.0 - (pos.x*pos.x/2.0) - (pos.y*pos.y/2.0) + (pos.x*pos.x*pos.y*pos.y / 3.0) );\n"
                                                     "  vec4 new_pos;\n"
                                                     "  new_pos.x = dx;\n"
                                                     "  new_pos.y = dy;\n"
                                                     "  new_pos.z = dz;\n"
                                                     "\n"
                                                     "  new_pos.w = pos.w;\n"
                                                     "  return new_pos;\n"
                                                     "}\n"
                                                     "void main(void) {\n"
                                                     "float mxAm = 0.5 * (1.0 + sin(ciElapsedSeconds));\n"
                                                     "vUv = ciTexCoord0;\n"
                                                     
                                                     "vec4 sphere_new_pos = sphere(ciPosition);\n"
                                                     "vec4 sphere_new_normal = sphere(vec4(ciNormal,1.0));\n"
                                                     

                                                     "\n"
                                                     "vec4 newPosition = mix(ciPosition,sphere_new_pos,mxAm);\n"
                                                     "vec3 newNormal = mix(ciNormal,vec3(sphere_new_normal),mxAm);\n"
                                                     "\n"
                                                     "	//vVertexOut.position = ciModelView * ciPosition;\n"
                                                     "	vVertexOut.position = ciModelView * newPosition;\n"
                                                     "	vVertexOut.normal = ciNormalMatrix * newNormal;\n"
                                                     "	vVertexOut.color = ciColor;\n"
                                                     "	//gl_Position = ciModelViewProjection * ciPosition;\n"
                                                     "	gl_Position = ciModelViewProjection * newPosition;\n"
                                                     "}\n"
                                                     )
                                             .fragment(
                                                       "#version 150\n"
                                                       "\n"
                                                       "in VertexData	{\n"
                                                       "	vec4 position;\n"
                                                       "	vec3 normal;\n"
                                                       "	vec4 color;\n"
                                                       "} vVertexIn;\n"
                                                       "\n"
                                                       "out vec4 oColor;\n"
                                                       "in vec2 vUv;\n"
                                                       "uniform sampler2D baseTexture;\n"
                                                       "\n"
                                                       "void main(void) {\n"
                                                       "	// set diffuse and specular colors\n"
                                                       "	vec3 cDiffuse = vVertexIn.color.rgb;\n"
                                                       "	vec3 cSpecular = vec3(0.3, 0.3, 0.3);\n"
                                                       "\n"
                                                       "	// light properties in view space\n"
                                                       "	vec3 vLightPosition = vec3(0.0, 0.0, 0.0);\n"
                                                       "\n"
                                                       "	// lighting calculations\n"
                                                       "	vec3 vVertex = vVertexIn.position.xyz;\n"
                                                       "	vec3 vNormal = normalize( vVertexIn.normal );\n"
                                                       "	vec3 vToLight = normalize( vLightPosition - vVertex );\n"
                                                       "	vec3 vToEye = normalize( -vVertex );\n"
                                                       "	vec3 vReflect = normalize( -reflect(vToLight, vNormal) );\n"
                                                       "\n"
                                                       "	// diffuse coefficient\n"
                                                       "	vec3 diffuse = max( dot( vNormal, vToLight ), 0.0 ) * cDiffuse;\n"
                                                       "\n"
                                                       "	// specular coefficient with energy conservation\n"
                                                       "	const float shininess = 20.0;\n"
                                                       "	const float coeff = (2.0 + shininess) / (2.0 * 3.14159265);\n"
                                                       "	vec3 specular = pow( max( dot( vReflect, vToEye ), 0.0 ), shininess ) * coeff * cSpecular;\n"
                                                       "\n"
                                                       "	// to conserve energy, diffuse and specular colors should not exceed one\n"
                                                       "	float maxDiffuse = max(diffuse.r, max(diffuse.g, diffuse.b));\n"
                                                       "	float maxSpecular = max(specular.r, max(specular.g, specular.b));\n"
                                                       "	float fConserve = 1.0 / max(1.0, maxDiffuse + maxSpecular);\n"
                                                       "\n"
                                                       "	// final color\n"
                                                       "	oColor.rgb = (diffuse + specular) * fConserve;\n"
                                                       "	//oColor.rgb = vec3(texture(baseTexture,vUv),0.0);\n"
                                                       "	oColor.a = 1.0;\n"
                                                       "}\n"
                                                       )
                                             );
	}
	catch( const std::exception& e ) {
		console() << e.what() << std::endl;
	}
}


void GeometryApp::createPlaneShader(void)
{
	try {
		mPlaneShader = gl::GlslProg::create( gl::GlslProg::Format()
			.vertex(
				"#version 150\n"
				"\n"
				"uniform mat4	ciModelViewProjection;\n"
				"uniform mat4	ciModelView;\n"
				"uniform mat3	ciNormalMatrix;\n"
                "uniform float	mixAmount;\n"
                "uniform vec3 worldUp;\n"
                "uniform float ciElapsedSeconds;\n"
                "uniform float angle_deg_max;\n"
                "uniform float height;\n"
                "uniform vec3 centerPoint;\n"
                "const float PI = 3.1415926535897932384626433832795;\n"
				"\n"
				"in vec4		ciPosition;\n"
				"in vec3		ciNormal;\n"
				"in vec4		ciColor;\n"
                "\n"
                "in vec2		ciTexCoord0;\n"
                "out vec2 vUv;\n"
                "uniform bool flag;\n"
                "uniform float move;\n"
				"\n"
				"out VertexData {\n"
				"	vec4 position;\n"
				"	vec3 normal;\n"
				"	vec4 color;\n"
				"} vVertexOut;\n"
				"\n"
                "\n"
				"void main(void) {\n"

                "float mxAm = 0.5 * (1.0 + sin(ciElapsedSeconds));\n"
                "vUv = ciTexCoord0;\n"
//                "vec3 goalPosition = 50.0 * vec3(0,ciTexCoord0.y,-ciTexCoord0.x) + vec3(-50.0,-100.0,100.0);\n"
                "vec3 goalPosition = 5.0 * vec3(-ciTexCoord0.x,ciTexCoord0.y,-ciTexCoord0.x) ;\n"
//                "vec4 newPosition = mix(ciPosition,vec4(goalPosition,1.0),mxAm);\n"
                  
        
                    ///*Oval Shaped
//                  "vec3 goalPosition = 5.0 * vec3(0,ciTexCoord0.y,0) ;\n"
                "vec4 newPosition;\n"
                "vec3 newNormal;\n"
                  "if(flag == true){\n"
                  "newPosition = mix(ciPosition,vec4(goalPosition,1.0),mxAm);\n"
                    "}\n"
                    "else{\n"
                    "newPosition = mix(ciPosition,vec4(goalPosition,1.0),move);\n"
                    "}\n"
                   //*/
                    
                    
//                "vec3 goalPosition;\n"
//                    "if(flag){goalPosition = 50.0 * vec3(0,0,-ciTexCoord0.y) + vec3(-50.0,-100.0,100.0);}\n"
//                    "else{goalPosition = 50.0 * vec3(0,0,-ciTexCoord0.y) + vec3(-50.0,-100.0,100.0);}\n"
                    
                

                "vec3 wU = cross(worldUp,vec3(newPosition));\n"
                "if(flag == true){\n"
                "newNormal = mix(ciNormal,wU,mxAm);\n"
                "}\n"
                "else{"
                "newNormal = mix(ciNormal,wU,move);\n"
                "}\n"
                "\n"
                "	//vVertexOut.position = ciModelView * ciPosition;\n"
                "	vVertexOut.position = ciModelView * newPosition;\n"
				"	vVertexOut.normal = ciNormalMatrix * newNormal;\n"
				"	vVertexOut.color = ciColor;\n"
				"	//gl_Position = ciModelViewProjection * ciPosition;\n"
                "	gl_Position = ciModelViewProjection * newPosition;\n"
				"}\n"
			)
			.fragment(
				"#version 150\n"
				"\n"
				"in VertexData	{\n"
				"	vec4 position;\n"
				"	vec3 normal;\n"
				"	vec4 color;\n"
				"} vVertexIn;\n"
				"\n"
				"out vec4 oColor;\n"
                "in vec2 vUv;\n"
                "uniform sampler2D baseTexture;\n"
				"\n"
				"void main(void) {\n"
				"	// set diffuse and specular colors\n"
				"	vec3 cDiffuse = vVertexIn.color.rgb;\n"
				"	vec3 cSpecular = vec3(0.3, 0.3, 0.3);\n"
				"\n"
				"	// light properties in view space\n"
				"	vec3 vLightPosition = vec3(0.0, 0.0, 0.0);\n"
				"\n"
				"	// lighting calculations\n"
				"	vec3 vVertex = vVertexIn.position.xyz;\n"
				"	vec3 vNormal = normalize( vVertexIn.normal );\n"
				"	vec3 vToLight = normalize( vLightPosition - vVertex );\n"
				"	vec3 vToEye = normalize( -vVertex );\n"
				"	vec3 vReflect = normalize( -reflect(vToLight, vNormal) );\n"
				"\n"
				"	// diffuse coefficient\n"
				"	vec3 diffuse = max( dot( vNormal, vToLight ), 0.0 ) * cDiffuse;\n"
				"\n"
				"	// specular coefficient with energy conservation\n"
				"	const float shininess = 20.0;\n"
				"	const float coeff = (2.0 + shininess) / (2.0 * 3.14159265);\n"
				"	vec3 specular = pow( max( dot( vReflect, vToEye ), 0.0 ), shininess ) * coeff * cSpecular;\n"
				"\n"
				"	// to conserve energy, diffuse and specular colors should not exceed one\n"
				"	float maxDiffuse = max(diffuse.r, max(diffuse.g, diffuse.b));\n"
				"	float maxSpecular = max(specular.r, max(specular.g, specular.b));\n"
				"	float fConserve = 1.0 / max(1.0, maxDiffuse + maxSpecular);\n"
				"\n"
				"	// final color\n"
				"	oColor.rgb = (diffuse + specular) * fConserve;\n"
                "	//oColor.rgb = vec3(texture(baseTexture,vUv),0.0);\n"
				"	oColor.a = 1.0;\n"
				"}\n"
			)
		);
	}
	catch( const std::exception& e ) {
		console() << e.what() << std::endl;
	}
}

void GeometryApp::createWireframeShader(void)
{
	try {
		mWireframeShader = gl::GlslProg::create( gl::GlslProg::Format()
			.vertex(
				"#version 150\n"
				"\n"
				"uniform mat4	ciModelViewProjection;\n"
				"in vec4		ciPosition;\n"
				"in vec4		ciColor;\n"
				"in vec2		ciTexCoord0;\n"
				"\n"
				"out VertexData {\n"
				"	vec4 color;\n"
				"	vec2 texcoord;\n"
				"} vVertexOut;\n"
				"\n"
				"void main(void) {\n"
				"	vVertexOut.color = ciColor;\n"
				"	vVertexOut.texcoord = ciTexCoord0;\n"
				"	gl_Position = ciModelViewProjection * ciPosition;\n"
				"}\n"
			) 
			.geometry(
				"#version 150\n"
				"\n"
				"layout (triangles) in;\n"
				"layout (triangle_strip, max_vertices = 3) out;\n"
				"\n"
				"uniform vec2 			uViewportSize;\n"
				"\n"
				"in VertexData	{\n"
				"	vec4 color;\n"
				"	vec2 texcoord;\n"
				"} vVertexIn[];\n"
				"\n"
				"out VertexData	{\n"
				"	noperspective vec3 distance;\n"
				"	vec4 color;\n"
				"	vec2 texcoord;\n"
				"} vVertexOut;\n"
				"\n"
				"void main(void)\n"
				"{\n"
				"	// taken from 'Single-Pass Wireframe Rendering'\n"
				"	vec2 p0 = uViewportSize * gl_in[0].gl_Position.xy / gl_in[0].gl_Position.w;\n"
				"	vec2 p1 = uViewportSize * gl_in[1].gl_Position.xy / gl_in[1].gl_Position.w;\n"
				"	vec2 p2 = uViewportSize * gl_in[2].gl_Position.xy / gl_in[2].gl_Position.w;\n"
				"\n"
				"	vec2 v0 = p2-p1;\n"
				"	vec2 v1 = p2-p0;\n"
				"	vec2 v2 = p1-p0;\n"
				"	float fArea = abs(v1.x*v2.y - v1.y * v2.x);\n"
				"\n"
				"	vVertexOut.distance = vec3(fArea/length(v0),0,0);\n"
				"	vVertexOut.color = vVertexIn[0].color;\n"
				"	vVertexOut.texcoord = vVertexIn[0].texcoord;\n"
				"	gl_Position = gl_in[0].gl_Position;\n"
				"	EmitVertex();\n"
				"\n"
				"	vVertexOut.distance = vec3(0,fArea/length(v1),0);\n"
				"	vVertexOut.color = vVertexIn[1].color;\n"
				"	vVertexOut.texcoord = vVertexIn[1].texcoord;\n"
				"	gl_Position = gl_in[1].gl_Position;\n"
				"	EmitVertex();\n"
				"\n"
				"	vVertexOut.distance = vec3(0,0,fArea/length(v2));\n"
				"	vVertexOut.color = vVertexIn[2].color;\n"
				"	vVertexOut.texcoord = vVertexIn[2].texcoord;\n"
				"	gl_Position = gl_in[2].gl_Position;\n"
				"	EmitVertex();\n"
				"\n"
				"	EndPrimitive();\n"
				"}\n"
			) 
			.fragment(
				"#version 150\n"
				"\n"
				"uniform float uBrightness;\n"
				"\n"
				"in VertexData	{\n"
				"	noperspective vec3 distance;\n"
				"	vec4 color;\n"
				"	vec2 texcoord;\n"
				"} vVertexIn;\n"
				"\n"
				"out vec4				oColor;\n"
				"\n"
				"void main(void) {\n"
				"	// determine frag distance to closest edge\n"
				"	float fNearest = min(min(vVertexIn.distance[0],vVertexIn.distance[1]),vVertexIn.distance[2]);\n"
				"	float fEdgeIntensity = exp2(-1.0*fNearest*fNearest);\n"
				"\n"
				"	// blend between edge color and face color\n"
				"	vec3 vFaceColor = vVertexIn.color.rgb;\n"
				"	vec3 vEdgeColor = vec3(0.2, 0.2, 0.2);\n"
				"	oColor.rgb = mix(vFaceColor, vEdgeColor, fEdgeIntensity) * uBrightness;\n"
				"	oColor.a = 0.65;\n"
				"}\n"
			)
		);
	}
	catch( const std::exception& e ) {
		console() << e.what() << std::endl;
	}
}

CINDER_APP_NATIVE( GeometryApp, RendererGl )
