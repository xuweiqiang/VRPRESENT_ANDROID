//
//  ESRender.h
//  LIbVRPresent
//
//  Created by weiqiang xu on 11/28/11.
//  Copyright (c) 2011 __MyCompanyName__. All rights reserved.
//
#ifndef _ESRENDER_H
#define _ESRENDER_H

#include "windows.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

typedef struct
{
	int left;
	int top;
	int right;
	int bottom;
}Rect;


typedef struct _vertexStruct
{
    GLfloat position[3];
    GLfloat color[4];
    GLfloat texCoord[2];
} vertexStruct;

typedef struct
{
    bool valid;
    vertexStruct parms[4];
    GLuint width;
    GLuint height;
    long format;

    GLuint miWidth;
    GLuint miHeight;
    Rect srcRect;
    Rect dstRect;
    GLuint textueId;
    bool hidden;
    GLfloat alpha;

    GLuint vertexbufferId;
    GLuint indexbufferId;

}STextureParams;
#define MAX_SURFACENUM  4


class ESRender
{
public:
	ESRender();
	virtual ~ESRender();

	GLuint createTextureWidth(GLuint width, GLuint height, long format);
	void DestroyTexture(GLuint textureId);
	void ShowHideTexture(GLuint textureId, bool bShow);
	void SetSrcRectTexture(GLuint textureId,Rect* pRect);
	void SetDestRectTexture(GLuint textureId,Rect* dstRect);
	void RenderTexture(GLuint textureId,  unsigned char* src_buffer);
	void SetAlphaTexture( GLuint textureId,float alpha);

	bool resizeFromLayer(int width, int height);
	int getWindowWidth(int* pWidth,int* pHeight);

	void SetBrightness(float brightness);
	void SetContrast(float contrast);
	GLuint getMiValue(GLuint width);
	GLfloat getMiPercent(GLuint width);
	void SetZoomMode(long mode);

private:
	GLuint loadShader(GLenum shaderType, const char* pSource);
	GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);

	void compileShaders();
	GLuint compileShaderCode(const char* shadercode,GLenum shaderType);



	GLint backingWidth;
	GLint backingHeight;

	GLuint _viewWidth;
	GLuint _viewHeight;

	GLuint _positionSlot;
	GLuint _colorSlot;
	GLuint _texCoordSlot;
	GLuint _textureUniform;

	GLuint _brightness;
	GLuint _contrast;

	float m_aspectRatio;

	STextureParams mSurfaces[MAX_SURFACENUM+2];
	bool mInit;
	GLuint m_programHandle;
	GLuint m_vertexShader;
	GLuint m_fragmentShader;

	GLuint _Y_TextureUniform;
	GLuint _U_TextureUniform;
	GLuint _V_TextureUniform;

	GLuint UTextureId;
	GLuint VTextureId;

};

#endif
