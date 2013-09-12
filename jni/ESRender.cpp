#include "ESRender.h"
#include "dp.h"


#ifndef MAKEFOURCC
#define MAKEFOURCC(c1,c2,c3,c4) ((c4)<<24 | (c3)<<16 | (c2)<<8 | (c1))
#endif

#define FIND_TEXTURE_ID_ORRETURN(i)    \
		for (i=0; i<MAX_SURFACENUM; i++) {  \
			if (mSurfaces[i].valid && mSurfaces[i].textueId == textureId) { \
				break;                      \
			}                               \
		}                                   \
		if (i>=MAX_SURFACENUM) {            \
			return;                         \
		}

const GLubyte indices[] = {
		0,1,2,
		2,3,0
};

#define YES true
#define NO false

static void printGLString(const char *name, GLenum s) {
	const char *v = (const char *) glGetString(s);
	LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
	for (GLint error = glGetError(); error; error
	= glGetError()) {
		LOGI("after %s() glError (0x%x)\n", op, error);
	}
}

static const char gVertexShader[] =
		"attribute vec4 vPosition;\n"
		"attribute vec2 aTextureCoord;\n"
		"varying vec2 vTextureCoord;\n"
		"void main() {\n"
		"  gl_Position = vPosition;\n"
		"  vTextureCoord = aTextureCoord;\n"
		"}\n";

static const char gFragmentShader[] =
		"precision mediump float;\n"
		"varying vec2 vTextureCoord;\n"
		"uniform sampler2D sTexture;\n"
		"void main() {\n"
		"  gl_FragColor = texture2D(sTexture, vTextureCoord);\n"
		"}\n";

GLuint gProgram;
GLuint gvPositionHandle;
GLuint maTextureHandle;
GLuint mTextureID;
const GLuint mTextureWidth = 1024;
const GLuint mTextureHeight = 1024;

const GLfloat gTriangleVertices[] = { 0.5f, -0.5f, 0.5f, 0.5f,
		-0.5f, 0.5f, -0.5f, -0.5f };

const GLfloat maTextureVertices[] = { 1.0f, 1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f };

unsigned char* mBuffer = 0;



ESRender::ESRender()
{
	memset(mSurfaces, 0, sizeof(mSurfaces));


	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &backingWidth);
	glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &backingHeight);

	compileShaders();

	for (int i=0; i<MAX_SURFACENUM+2; i++)
	{
		glActiveTexture(GL_TEXTURE0+i);
		glGenTextures(1, &mSurfaces[i].textueId);
		glBindTexture(GL_TEXTURE_2D, mSurfaces[i].textueId);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


		glGenBuffers(1, &mSurfaces[i].vertexbufferId);
		glGenBuffers(1, &mSurfaces[i].indexbufferId);

	}


	UTextureId = MAX_SURFACENUM;
	VTextureId = MAX_SURFACENUM+1;

	mInit = true;

}
//ESRender::ESRender()
//{
//	printGLString("Version", GL_VERSION);
//	printGLString("Vendor", GL_VENDOR);
//	printGLString("Renderer", GL_RENDERER);
//	printGLString("Extensions", GL_EXTENSIONS);
//
//	gProgram = createProgram(gVertexShader, gFragmentShader);
//	if (!gProgram) {
//		DP("Could not create program.");
//		return;
//	}
//
//	glGenTextures(1, &mTextureID);
//	glBindTexture(GL_TEXTURE_2D, mTextureID);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
//			GL_LINEAR);
//	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
//			GL_LINEAR);
//
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
//			GL_REPEAT);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
//			GL_REPEAT);
//
//
//	gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
//	checkGlError("glGetAttribLocation");
//	LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
//			gvPositionHandle);
//
//	maTextureHandle = glGetAttribLocation(gProgram, "aTextureCoord");
//	checkGlError("glGetAttribLocation");
//	LOGI("glGetAttribLocation(\"aTextureCoord\") = %d\n",
//			maTextureHandle);
//
//	mBuffer = (unsigned char*)malloc(mTextureWidth*mTextureHeight*2);
//
//
//	DP("init success.");
//
//}
//
ESRender::~ESRender()
{

}


GLuint ESRender::createTextureWidth(GLuint width, GLuint height, long format)
{
	//get a texture
	int emptySurfaceId = 0;
	for(emptySurfaceId=0;emptySurfaceId<MAX_SURFACENUM;emptySurfaceId++)
	{
		if (mSurfaces[emptySurfaceId].valid == false) {
			break;
		}
	}

	if (emptySurfaceId >= MAX_SURFACENUM)
		return 0;


	mSurfaces[emptySurfaceId].width = width;
	mSurfaces[emptySurfaceId].height = height;
	mSurfaces[emptySurfaceId].format = format;
	mSurfaces[emptySurfaceId].miWidth   = getMiValue(width);
	mSurfaces[emptySurfaceId].miHeight    = getMiValue(height);
	mSurfaces[emptySurfaceId].alpha = 1.0f;
	mSurfaces[emptySurfaceId].hidden = true;

	//
	GLfloat widthPercent =  getMiPercent(width);
	GLfloat heightPercent =  getMiPercent(height);

	vertexStruct verticesTmp[] = {
			{{1,-1,0},{1,0,0,1.0},{1.0,1.0}},
			{{1,1,0},{0,1,0,1.0},{1.0,0.0}},
			{{-1,1,0},{0,0,1,1.0},{0.0,0.0}},
			{{-1,-1,0},{0,0,0,1.0},{0.0,1.0}}
	};


	for(int i=0;i<sizeof(verticesTmp)/sizeof(verticesTmp[0]);i++)
	{
		verticesTmp[i].texCoord[0] *= widthPercent;
		verticesTmp[i].texCoord[1] *= heightPercent;
		verticesTmp[i].position[2] = emptySurfaceId;
	}
	memcpy(mSurfaces[emptySurfaceId].parms, verticesTmp, sizeof(verticesTmp));


	unsigned char* pFlushBuff = (unsigned char*)malloc(mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight*4);

	glBindTexture(GL_TEXTURE_2D, mSurfaces[emptySurfaceId].textueId);
	if (mSurfaces[emptySurfaceId].format == MAKEFOURCC('R','5','6','5')) {
		memset(pFlushBuff, 0, mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight*2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mSurfaces[emptySurfaceId].miWidth, mSurfaces[emptySurfaceId].miHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[emptySurfaceId].miWidth, mSurfaces[emptySurfaceId].miHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, pFlushBuff);
	}
	else if (mSurfaces[emptySurfaceId].format == MAKEFOURCC('X','R','G','B')) {
		memset(pFlushBuff, 0, mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight*4);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,  mSurfaces[emptySurfaceId].miWidth, mSurfaces[emptySurfaceId].miHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[emptySurfaceId].miWidth, mSurfaces[emptySurfaceId].miHeight, GL_RGBA, GL_UNSIGNED_BYTE, pFlushBuff);
	}
	else if (mSurfaces[emptySurfaceId].format == MAKEFOURCC('R','5','5','5')) {
		memset(pFlushBuff, 0, mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight*2);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSurfaces[emptySurfaceId].miWidth, mSurfaces[emptySurfaceId].miHeight, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[emptySurfaceId].miWidth, mSurfaces[emptySurfaceId].miHeight, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, pFlushBuff);
	}
	else if (mSurfaces[emptySurfaceId].format == MAKEFOURCC('Y','V','1','2')) {
		memset(pFlushBuff, 0x00, mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight);
		memset(pFlushBuff+mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight
				, 0x80, mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight/2);

		glBindTexture (GL_TEXTURE_2D, mSurfaces[emptySurfaceId].textueId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mSurfaces[emptySurfaceId].miWidth,
				mSurfaces[emptySurfaceId].miHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[emptySurfaceId].miWidth,
				mSurfaces[emptySurfaceId].miHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, pFlushBuff);

		glBindTexture (GL_TEXTURE_2D, mSurfaces[UTextureId].textueId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mSurfaces[emptySurfaceId].miWidth>>1,
				mSurfaces[emptySurfaceId].miHeight>>1, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[emptySurfaceId].miWidth>>1,
				mSurfaces[emptySurfaceId].miHeight>>1, GL_LUMINANCE, GL_UNSIGNED_BYTE,
				pFlushBuff+mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight);

		glBindTexture (GL_TEXTURE_2D, mSurfaces[VTextureId].textueId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, mSurfaces[emptySurfaceId].miWidth>>1, mSurfaces[emptySurfaceId].miHeight>>1,
				0, GL_LUMINANCE, GL_UNSIGNED_BYTE, 0);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[emptySurfaceId].miWidth>>1, mSurfaces[emptySurfaceId].miHeight>>1,
				GL_LUMINANCE, GL_UNSIGNED_BYTE, pFlushBuff+mSurfaces[emptySurfaceId].miWidth*mSurfaces[emptySurfaceId].miHeight*5/4);
	}

	free(pFlushBuff);

	glBindBuffer(GL_ARRAY_BUFFER, mSurfaces[emptySurfaceId].vertexbufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mSurfaces[emptySurfaceId].parms), mSurfaces[emptySurfaceId].parms, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSurfaces[emptySurfaceId].indexbufferId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	mSurfaces[emptySurfaceId].valid = true;

	return mSurfaces[emptySurfaceId].textueId;
}
void ESRender::DestroyTexture(GLuint textureId)
{
	int emptySurfaceId = 0;
	for(emptySurfaceId=0;emptySurfaceId<MAX_SURFACENUM;emptySurfaceId++)
	{
		if (mSurfaces[emptySurfaceId].textueId == textureId) {
			break;
		}
	}

	if (emptySurfaceId >= MAX_SURFACENUM)
		return;

	mSurfaces[emptySurfaceId].valid = false;


}
void ESRender::ShowHideTexture(GLuint textureId, bool bShow)
{
	int i;
	FIND_TEXTURE_ID_ORRETURN(i)
	mSurfaces[i].hidden = !bShow;
	return;
}

void ESRender::SetSrcRectTexture(GLuint textureId,Rect* pRect)
{
	int i;
	FIND_TEXTURE_ID_ORRETURN(i)
	mSurfaces[i].srcRect = *pRect;


	GLfloat widthPercent =  getMiPercent(mSurfaces[i].width);
	GLfloat heightPercent = getMiPercent(mSurfaces[i].height);

	float cropWidth = (mSurfaces[i].srcRect.right - mSurfaces[i].srcRect.left)/
			(float)(mSurfaces[i].width);
	float cropHeight = (mSurfaces[i].srcRect.bottom - mSurfaces[i].srcRect.top)/
			(float)(mSurfaces[i].height);


	mSurfaces[i].parms[0].texCoord[0] = widthPercent*(1.0f - cropWidth)/2 + widthPercent*cropWidth;
	mSurfaces[i].parms[0].texCoord[1] = heightPercent*(1.0f- cropHeight)/2 + heightPercent*cropHeight;

	mSurfaces[i].parms[1].texCoord[0] = widthPercent*(1.0f - cropWidth)/2 + widthPercent*cropWidth;
	mSurfaces[i].parms[1].texCoord[1] = heightPercent*(1.0f - cropHeight)/2;

	mSurfaces[i].parms[2].texCoord[0] = widthPercent*(1.0f - cropWidth)/2;
	mSurfaces[i].parms[2].texCoord[1] = heightPercent*(1.0f- cropHeight)/2;

	mSurfaces[i].parms[3].texCoord[0] = widthPercent*(1.0f -cropWidth)/2;
	mSurfaces[i].parms[3].texCoord[1] = heightPercent*(1.0f- cropHeight)/2+ heightPercent*cropHeight;


	glBindBuffer(GL_ARRAY_BUFFER, mSurfaces[i].vertexbufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mSurfaces[i].parms), mSurfaces[i].parms, GL_STATIC_DRAW);

	return;
}

void ESRender::SetDestRectTexture(GLuint textureId,Rect* dstRect)
{
	if (dstRect->left > backingWidth || dstRect->right > backingWidth
			|| dstRect->top > backingHeight || dstRect->bottom > backingHeight) {
		return;
	}
	int i;
	FIND_TEXTURE_ID_ORRETURN(i)
	mSurfaces[i].dstRect = *dstRect;

	float left,top,right,bottom;
	left = mSurfaces[i].dstRect.left/(float)backingWidth;
	top = mSurfaces[i].dstRect.top/(float)backingHeight;
	right = mSurfaces[i].dstRect.right/(float)backingWidth;
	bottom = mSurfaces[i].dstRect.bottom/(float)backingHeight;

	//Convert cordination system
	left = left*2-1;
	right = right*2-1;
	top = (top*2 - 1)*-1;
	bottom = (bottom*2 -1)*-1;



	mSurfaces[i].parms[0].position[0] = right;
	mSurfaces[i].parms[0].position[1] = bottom;

	mSurfaces[i].parms[1].position[0] = right;
	mSurfaces[i].parms[1].position[1] = top;

	mSurfaces[i].parms[2].position[0] = left;
	mSurfaces[i].parms[2].position[1] = top;

	mSurfaces[i].parms[3].position[0] = left;
	mSurfaces[i].parms[3].position[1] = bottom;

	glBindBuffer(GL_ARRAY_BUFFER, mSurfaces[i].vertexbufferId);
	glBufferData(GL_ARRAY_BUFFER, sizeof(mSurfaces[i].parms), mSurfaces[i].parms, GL_STATIC_DRAW);

	return;
}

void ESRender::RenderTexture(GLuint textureId,  unsigned char* src_buffer)
{
	if (mInit == NO) {
		return;
	}
	if (textureId == 0) {
		return;
	}

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	int i = 0;
	for (i=0; i<MAX_SURFACENUM; i++) {
		if (mSurfaces[i].valid == NO) {
			continue;
		}
		if (mSurfaces[i].hidden) {
			continue;
		}
		glBindBuffer(GL_ARRAY_BUFFER, mSurfaces[i].vertexbufferId);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mSurfaces[i].indexbufferId);

		glVertexAttribPointer(_positionSlot, 3, GL_FLOAT, GL_FALSE, sizeof(vertexStruct), 0);
		glVertexAttribPointer(_colorSlot, 4, GL_FLOAT, GL_FALSE, sizeof(vertexStruct),(GLvoid*)(sizeof(GLfloat)*3));
		glVertexAttribPointer(_texCoordSlot, 2, GL_FLOAT, GL_FALSE, sizeof(vertexStruct), (GLvoid*)(sizeof(GLfloat)*7));

		glBindTexture (GL_TEXTURE_2D, mSurfaces[i].textueId);
		if (mSurfaces[i].format == MAKEFOURCC('Y','V','1','2')) {
			glBindTexture (GL_TEXTURE_2D, mSurfaces[UTextureId].textueId);
			glBindTexture (GL_TEXTURE_2D, mSurfaces[VTextureId].textueId);

		}

		if (textureId == mSurfaces[i].textueId) {
			if (mSurfaces[i].format == MAKEFOURCC('R','5','6','5')) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, mSurfaces[i].miWidth, mSurfaces[i].miHeight, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, 0);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[i].miWidth, mSurfaces[i].miHeight, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, src_buffer);
			}
			else if (mSurfaces[i].format == MAKEFOURCC('X','R','G','B')) {
				glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA,  mSurfaces[i].miWidth, mSurfaces[i].miHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[i].miWidth, mSurfaces[i].miHeight, GL_RGBA, GL_UNSIGNED_BYTE, src_buffer);
			}
			else if (mSurfaces[i].format == MAKEFOURCC('R','5','5','5')) {
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mSurfaces[i].miWidth, mSurfaces[i].miHeight, 0, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, 0);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[i].miWidth, mSurfaces[i].miHeight, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1, src_buffer);
			}
			else if (mSurfaces[i].format == MAKEFOURCC('Y','V','1','2')) {
				glBindTexture (GL_TEXTURE_2D, mSurfaces[i].textueId);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[i].width, mSurfaces[i].height, GL_LUMINANCE, GL_UNSIGNED_BYTE, src_buffer);


				glBindTexture (GL_TEXTURE_2D, mSurfaces[UTextureId].textueId);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[i].width>>1, mSurfaces[i].height>>1, GL_LUMINANCE,
						GL_UNSIGNED_BYTE, src_buffer+mSurfaces[i].width*mSurfaces[i].height);


				glBindTexture (GL_TEXTURE_2D, mSurfaces[VTextureId].textueId);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mSurfaces[i].width>>1, mSurfaces[i].height>>1, GL_LUMINANCE,
						GL_UNSIGNED_BYTE, src_buffer+mSurfaces[i].width*mSurfaces[i].height*5/4);

				glUniform1i(_Y_TextureUniform, i);
				glUniform1i(_U_TextureUniform, UTextureId);
				glUniform1i(_V_TextureUniform, VTextureId);
			}
		}
		glUniform1i(_textureUniform, i);
		glDrawElements(GL_TRIANGLES, sizeof(indices)/sizeof(indices[0]), GL_UNSIGNED_BYTE, 0);

	}
	//    GLenum erro = glGetError();
	//    if (erro!= GL_NO_ERROR) {
	//        NSLog(@"OPENGLES RENDER ERROR:0x%x \n",erro);
	//    }
	return;

}
//void ESRender::RenderTexture(GLuint textureId,  unsigned char* src_buffer)
//{
//	static float grey;
//	grey += 0.01f;
//	if (grey > 1.0f) {
//		grey = 0.0f;
//	}
//	glClearColor(grey, grey, grey, 1.0f);
//	checkGlError("glClearColor");
//	glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
//	checkGlError("glClear");
//
//	glActiveTexture(GL_TEXTURE0);
//
//	glUseProgram(gProgram);
//	checkGlError("glUseProgram");
//
//	glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
//	checkGlError("glVertexAttribPointer");
//	glEnableVertexAttribArray(gvPositionHandle);
//
//	glVertexAttribPointer(maTextureHandle, 2, GL_FLOAT, GL_FALSE, 0, maTextureVertices);
//	checkGlError("glVertexAttribPointer");
//
//	glEnableVertexAttribArray(maTextureHandle);
//	checkGlError("glEnableVertexAttribArray");
//
//	memset(mBuffer, rand(), mTextureWidth*mTextureHeight*2);
//	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB, mTextureWidth,mTextureHeight,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,mBuffer);
//	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
//	checkGlError("glDrawArrays");
//
//}
void ESRender::SetAlphaTexture( GLuint textureId,float alpha)
{
	int i;
	FIND_TEXTURE_ID_ORRETURN(i)
	mSurfaces[i].alpha = alpha;
	return;
}

bool ESRender::resizeFromLayer(int width, int height)
{
	backingWidth = width;
	backingHeight = height;
	glViewport(0, 0, backingWidth, backingHeight);
//	checkGlError("glViewport");
	return YES;
}
int ESRender::getWindowWidth(int* pWidth,int* pHeight)
{
	if (mInit == false) {
		return -1;
	}
	*pWidth = backingWidth;
	*pHeight = backingHeight;
	return 0;
}

void ESRender::SetBrightness(float brightness)
{
	glUniform1f(_brightness, brightness);

}
void ESRender::SetContrast(float contrast)
{
	glUniform1f(_contrast, contrast);
}

GLuint ESRender::getMiValue(GLuint width)
{
	if (width == 0) {
		return -1;
	}
	GLuint mi =0;

	switch (width) {
	case 2:
	case 4:
	case 8:
	case 16:
	case 32:
	case 64:
	case 128:
	case 256:
	case 512:
	case 1024:
	case 2048:
		return width;
	default:
		break;
	}

	while(width >0)
	{
		mi++;
		width >>= 1;
	}
	return (2 <<(mi-1));
}

GLfloat ESRender::getMiPercent(GLuint width)
{
	GLuint mivalue = getMiValue(width);

	return (GLfloat)((GLfloat)width/(GLfloat)mivalue);
}

void ESRender::SetZoomMode(long mode)
{

}

GLuint ESRender::loadShader(GLenum shaderType, const char* pSource)
{
	GLuint shader = glCreateShader(shaderType);
	if (shader) {
		glShaderSource(shader, 1, &pSource, NULL);
		glCompileShader(shader);
		GLint compiled = 0;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			GLint infoLen = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				char* buf = (char*) malloc(infoLen);
				if (buf) {
					glGetShaderInfoLog(shader, infoLen, NULL, buf);
					DP("Could not compile shader %d:\n%s\n",
							shaderType, buf);
					free(buf);
				}
				glDeleteShader(shader);
				shader = 0;
			}
		}
	}
	return shader;
}
GLuint ESRender::createProgram(const char* pVertexSource, const char* pFragmentSource)
{
	GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
	if (!vertexShader) {
		return 0;
	}

	GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
	if (!pixelShader) {
		return 0;
	}

	GLuint program = glCreateProgram();
	if (program) {
		glAttachShader(program, vertexShader);
		checkGlError("glAttachShader");
		glAttachShader(program, pixelShader);
		checkGlError("glAttachShader");
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint bufLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
			if (bufLength) {
				char* buf = (char*) malloc(bufLength);
				if (buf) {
					glGetProgramInfoLog(program, bufLength, NULL, buf);
					DP("Could not link program:\n%s\n", buf);
					free(buf);
				}
			}
			glDeleteProgram(program);
			program = 0;
		}
	}
	return program;

}

GLuint ESRender::compileShaderCode(const char* shadercode,GLenum shaderType)
{
	GLuint shaderHandle = glCreateShader(shaderType);

	glShaderSource(shaderHandle, 1, &shadercode, NULL);

	glCompileShader(shaderHandle);
	GLint compileSuccess;
	glGetShaderiv(shaderHandle, GL_COMPILE_STATUS, &compileSuccess);
	if(compileSuccess == GL_FALSE)
	{
		GLint infoLen = 0;
		glGetShaderiv(shaderHandle, GL_INFO_LOG_LENGTH, &infoLen);
		if (infoLen) {
			char* buf = (char*) malloc(infoLen);
			if (buf) {
				glGetShaderInfoLog(shaderHandle, infoLen, NULL, buf);
				LOGE("Could not compile shader %d:\n%s\n",
						shaderType, buf);
				free(buf);
			}
		}
		glDeleteShader(shaderHandle);
		return 0;
	}
	return shaderHandle;

}

void ESRender::compileShaders()
{
	const char vertexShaderCode[] =        "   \
	        attribute vec4 Position;        \
	        attribute vec4 SourceColor;     \
	        attribute vec2 TexCooordIn;     \
	        varying vec4 DestinationColor;  \
	        varying vec2 TexCoordOut;       \
	        void main()                     \
	        {                               \
	            DestinationColor = SourceColor; \
	            gl_Position = Position; \
	            TexCoordOut = TexCooordIn;  \
	        }   \
	    ";

	const char fragmengShaderCode_RGB565_MAIN[] =        "   \
	    varying lowp vec4 DestinationColor; \
	    varying lowp vec2 TexCoordOut;  \
	    uniform sampler2D main_texture;    \
	    uniform sampler2D Y_texture;    \
	    uniform sampler2D U_texture;    \
	    uniform sampler2D V_texture;    \
	    uniform sampler2D UV_texture;    \
	    uniform lowp float Brightness;    \
	    uniform lowp float Contrast;    \
	    uniform bool bUsingYUV;    \
	    void main() \
	    {   \
	    mediump vec3 yuv;       \
	    lowp vec3 rgb;  \
	    gl_FragColor = texture2D(main_texture,TexCoordOut);    \
	    gl_FragColor.rgb /= gl_FragColor.a;\
	    gl_FragColor.rgb = ((gl_FragColor.rgb - 0.5) * Contrast) + 0.5;   \
	    gl_FragColor.rgb += Brightness;\
	    gl_FragColor.rgb *= gl_FragColor.a;\
	    }   \
	    ";


	const char fragmengShaderCode[] =        "   \
	        varying lowp vec4 DestinationColor; \
	        varying lowp vec2 TexCoordOut;  \
	        uniform sampler2D main_texture;    \
	        uniform sampler2D Y_texture;    \
	        uniform sampler2D U_texture;    \
	        uniform sampler2D V_texture;    \
	        uniform lowp float Brightness;    \
	        uniform lowp float Contrast;    \
	        const highp mat3 yuv2rgb = mat3(      1,       1,       1,   \
	            0, -.21482, 2.12798,    \
	            1.28033, -.38059,       0);  \
	        void main() \
	        {   \
	        mediump vec3 yuv;       \
	        lowp vec3 rgb;  \
	        if(texture2D(Y_texture, TexCoordOut) == texture2D(main_texture,TexCoordOut))   \
	        {           \
	            yuv.x = texture2D(Y_texture, TexCoordOut).r;     \
	            yuv.y = texture2D(V_texture, TexCoordOut).r - 0.5;     \
	            yuv.z = texture2D(U_texture, TexCoordOut).r - 0.5;     \
	            rgb =  yuv2rgb* yuv;   \
	            gl_FragColor = vec4(rgb, 1.0);    \
	        }           \
	        else        \
	        {       \
	            gl_FragColor = texture2D(main_texture,TexCoordOut);    \
	        }   \
	        gl_FragColor.rgb /= gl_FragColor.a;\
	        gl_FragColor.rgb = ((gl_FragColor.rgb - 0.5) * Contrast) + 0.5;   \
	        gl_FragColor.rgb += Brightness;\
	        gl_FragColor.rgb *= gl_FragColor.a;\
	        }   \
	        ";

	m_vertexShader = compileShaderCode(vertexShaderCode,GL_VERTEX_SHADER);

	m_fragmentShader = compileShaderCode(fragmengShaderCode,GL_FRAGMENT_SHADER);

	m_programHandle = glCreateProgram();
	glAttachShader(m_programHandle, m_vertexShader);
	glAttachShader(m_programHandle, m_fragmentShader);
	glLinkProgram(m_programHandle);

	GLint linkSuccess;
	glGetProgramiv(m_programHandle, GL_LINK_STATUS, &linkSuccess);
	if(linkSuccess == GL_FALSE)
	{
		GLint infoLen = 0;
		glGetProgramiv(m_programHandle, GL_INFO_LOG_LENGTH, &infoLen);
		if(infoLen > 1)
		{
			char* infoLog = (char*)malloc(sizeof(char) * infoLen);
			glGetProgramInfoLog(m_programHandle, infoLen, NULL, infoLog);
			DP("Error linking program:\n%s\n", infoLog);
			free(infoLog);
		}
		glDeleteShader(m_vertexShader);
		glDeleteShader(m_fragmentShader);
		glDeleteProgram(m_programHandle);
		return;
	}
	glUseProgram(m_programHandle);

	_positionSlot = glGetAttribLocation(m_programHandle, "Position");
	_colorSlot
	= glGetAttribLocation(m_programHandle, "SourceColor");

	_texCoordSlot = glGetAttribLocation(m_programHandle, "TexCooordIn");
	_textureUniform = glGetUniformLocation(m_programHandle, "main_texture");
	_Y_TextureUniform = glGetUniformLocation(m_programHandle, "Y_texture");
	_U_TextureUniform = glGetUniformLocation(m_programHandle, "U_texture");
	_V_TextureUniform = glGetUniformLocation(m_programHandle, "V_texture");


	_brightness = glGetUniformLocation(m_programHandle, "Brightness");
	float default_brightness = 0.0f;
	glUniform1f(_brightness, default_brightness);

	_contrast = glGetUniformLocation(m_programHandle, "Contrast");
	float default_contrast = 1.0f;
	glUniform1f(_contrast, default_contrast);


	glEnableVertexAttribArray(_positionSlot);
	glEnableVertexAttribArray(_colorSlot);
	glEnableVertexAttribArray(_texCoordSlot);

	glEnable(GL_BLEND);
	//    glBlendFunc(GL_CONSTANT_ALPHA, GL_ONE_MINUS_CONSTANT_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	DP("compile shader success \n");

}


