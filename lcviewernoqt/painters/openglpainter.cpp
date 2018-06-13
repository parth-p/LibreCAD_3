#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdio.h>
#include <GL/glut.h>
#include <string.h>
#include "openglpainter.h"

using namespace LCViewer;

const char* vertexShaderCode =
    "#version 130\r\n"
    ""
    "in vec2 pos"
    "in vec4 color"
    "out vec4 oColor"
    ""
    "void main()"
    "{"
    "   oColor=color"
    "   gl_Position = vec4(pos,0.0,1.0)"
    "}";

const char* fragmentShaderCode =
    "#version 130\r\n"
    ""
    "in vec4 oColor"
    "out vec4 theColor"
    ""
    "void main()"
    "{"
    "   theColor=oColor"
    "}";

OpenGLPainter::OpenGLPainter(unsigned int width, unsigned int height):
    _selectedPattern(-1),
    _constantLineWidth(true) {

	glClearColor(0,0,0,1);

	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void OpenGLPainter::move_to(double x,double y) {
    _pos = lc::geo::Coordinate(x, y);
}

void OpenGLPainter::line_to(double x,double y) {
	GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {
        _pos.x(), _pos.y(),
        x, y
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);  

    glDrawArrays(GL_LINES, 0, 2);

    _pos = lc::geo::Coordinate(x, y);
}

void OpenGLPainter::rectangle(double x,double y,double w,double h) {
    _path.clear();
    _path.emplace_back(x + w, y);
    _path.emplace_back(x + w, y + h);
    _path.emplace_back(x, y + h);
    _path.emplace_back(x, y);
    
	GLfloat vertices[] = {
    	_path[0].x(), _path[0].y(), // Bottom-right
        _path[1].x(), _path[1].y(), // Top-right
        _path[2].x(), _path[2].y(), // Top-left
        _path[3].x(), _path[3].y(), // Bottom-left
    };

    // Create an element array
    GLuint vbo;
    glGenBuffers(1, &vbo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);
    
    GLuint elements[] = {
        0, 1, 2,
        2, 3, 0
    };

    GLuint ebo;
    glGenBuffers(1, &ebo);    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void OpenGLPainter::save() {
	glPushMatrix();
}

void OpenGLPainter::restore() {
	glPopMatrix();
}

void OpenGLPainter::enable_antialias() {
	glEnable(GL_MULTISAMPLE);
}

void OpenGLPainter::disable_antialias() {
	glDisable(GL_MULTISAMPLE);
}

void OpenGLPainter::source_rgb(double r, double g, double b) {
	glColor3d(r,g,b);
    _selectedPattern = -1;
}

void OpenGLPainter::source_rgba(double r, double g, double b, double a) {
	glColor4d(r,g,b,a);
    _selectedPattern = -1;
}

void OpenGLPainter::translate(double x, double y) {
    glTranslated(x, - y, 0);
}

void OpenGLPainter::getTranslate(double* x, double* y) {
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    (*x) = modelview[3];
    (*y) = - modelview[7];
}

void OpenGLPainter::line_width(double lineWidth) {
    if (_constantLineWidth) {
        glLineWidth((lineWidth + _lineWidthCompensation) / scale());
    } 
    else {
        glLineWidth(_lineWidthCompensation);
    }

    _lineWidth = lineWidth;
}

void OpenGLPainter::lineWidthCompensation(double lwc) {
    _lineWidthCompensation = lwc;
}

void OpenGLPainter::clear(double r, double g, double b) {
    glClearColor(r, g, b, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLPainter::clear(double r, double g, double b, double a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

// void OpenGLPainter::text(double x, double y, char *str) {
// 	glRasterPos2f(x,y);
// 	int len=(int) strlen(str);
// 	for (int i=0;i<len;i++)
// 		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,str[i]);
// }

void OpenGLPainter::scale(double s) {
	glScaled(s, s, s);
}

double OpenGLPainter::scale() {
    GLdouble modelview[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);

    return modelview[0];
}

void OpenGLPainter::fill() {
    if(_path.size() == 4 && _selectedPattern != -1) {
        auto pattern = _patterns.at(_selectedPattern);

        auto startColor = pattern.colors.begin()->second;
        auto endColor = pattern.colors.end()->second;
        auto middleColor = lc::Color(
            (startColor.red() + endColor.red()) / 2,
            (startColor.green() + endColor.green()) / 2,
            (startColor.blue() + endColor.blue()) / 2,
            (startColor.alpha() + endColor.alpha()) / 2
        );

        GLfloat vertices[] = {
            _path[0].x(), _path[0].y(),
            startColor.red(), startColor.green(), startColor.blue(), startColor.alpha(), // Top-left
            _path[1].x(), _path[1].y(), // Top-right
            startColor.red(), middleColor.green(), middleColor.blue(), middleColor.alpha(),
            _path[2].x(), _path[2].y(),
            startColor.red(), endColor.green(), endColor.blue(), endColor.alpha(), // Bottom-right
            _path[3].x(), _path[3].y(),
            startColor.red(), middleColor.green(), middleColor.blue(), middleColor.alpha() // Bottom-left
        };

        // Create an element array
        GLuint vbo;
        glGenBuffers(1, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(float)*6,0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,sizeof(float)*6, (char*)(sizeof(float)*2));
        
        GLuint elements[] = {
            0, 1, 2,
            2, 3, 0
        };

        GLuint ebo;
        glGenBuffers(1, &ebo);    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

        GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
        GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

        glShaderSource(vertexShaderID, 1, &vertexShaderCode, NULL);
        glShaderSource(fragmentShaderID, 1, &fragmentShaderCode, NULL);

        glCompileShader(vertexShaderID);
        glCompileShader(fragmentShaderID);

        GLuint shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShaderID);
        glAttachShader(shaderProgram, fragmentShaderID);
        glBindFragDataLocation(shaderProgram, 0, "theColor");
        glLinkProgram(shaderProgram);
        glUseProgram(shaderProgram);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
    else {
        GLfloat vertices[2*_path.size()];
        for(int i=0;i<_path.size();i++) {
            vertices[2*i] = _path[i].x();
            vertices[2*i+1] = _path[i].y();
        }

        GLuint vbo;
        glGenBuffers(1, &vbo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);
        
        GLuint elements[] = {
            0, 1, 2,
            2, 3, 0
        };

        GLuint ebo;
        glGenBuffers(1, &ebo);    
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}

void OpenGLPainter::set_dash(const double* dashes, const int num_dashes, double offset, bool scaled) {
    memcpy(&scaledDashes, dashes, num_dashes * sizeof(double));
    double _scale = scale();
    len_dash = num_dashes;
    if (scaled) {
        for (short c = 0; c < num_dashes; c++) {
            scaledDashes[c] = scaledDashes[c] / _scale;
        }
    }
}

void OpenGLPainter::stroke() {
    int flag=0;
    if(_path.size()==prev_len) {
        for (int i = 0; i < prev_len; i++) {
            if(prev_path[2*i]!=_path[i].x() || prev_path[2*i+1]!=_path[i].y())
                flag=1;
        }
    }
    else
        flag=1;

    glLineWidth(_lineWidth);
    int len = _path.size();

    int side=0;
    double left=0;
    int type=0;

    for (side = 0; side < len; side++) {
        double a = _path[side].x(),b = _path[side].y();
        double c = _path[side+1].x(),d = _path[side+1].y();
        move_to(a,b);

        double hyp = sqrt((d-b)*(d-b) + (c-a)*(c-a));
        double sin = (d-b)/hyp, cos = (c-a)/hyp;

        if(left>=0) {
            a+=left*cos;
            b+=left*sin;
            if(type%2==0) {
                if(flag==1)
                    line_to(a,b);
                else
                    line(a,b);
            }
            else
                move_to(a,b);
        }

        double period = 0;
        for (int i = 0; i < len_dash; i++)
            period += scaledDashes[i];

        double time = hyp/period;

        for (int i = 0; i < (time+1)*len_dash; i++) {
            a+=scaledDashes[i]*cos;
            b+=scaledDashes[i]*sin;
            if((sin>0 && b<=d) || (sin<=0 && b>=d)) {
                if(i%2==0) {
                    if(flag==1)
                        line_to(a,b);
                    else
                        line(a,b);
                }
                else
                    move_to(a,b);
            }
            else {
                if(i%2==0) {
                    if(flag==1)
                        line_to(a,b);
                    else
                        line(a,b);
                }
                else
                    move_to(c,d);
                left = sqrt((a-c)*(a-c) + (b-d)*(b-d));
                type = i;
                break;
            }
        }
    }

    if(flag==1) {
        for (int i = 0; i < _path.size(); i++) {
            prev_path[2*i]=_path[i].x();
            prev_path[2*i+1]=_path[i].y();
        }
        prev_len=_path.size();
    }
}

void OpenGLPainter::line(double x,double y) {
    GLuint vbo;
    glGenBuffers(1, &vbo);

    GLfloat vertices[] = {
        _pos.x(), _pos.y(),
        x, y
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,0,0);  
    glDrawArrays(GL_LINES, 0, 2);
}

void OpenGLPainter::user_to_device(double* x, double* y) {
    GLdouble z = 0;

    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    (*y) = viewport[3] - *y;

    gluProject(*x, *y, z, modelview, projection, viewport, x, y, &z);
}

void OpenGLPainter::device_to_user(double* x, double* y) {
    GLint viewport[4];
    GLdouble modelview[16];
    GLdouble projection[16];

    GLdouble z = 0;

    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    (*y) = viewport[3] - *y;

    gluUnProject(*x, *y, z, modelview, projection, viewport, x, y, &z);
}

void OpenGLPainter::user_to_device_distance(double* dx, double* dy) {
    double originX = 0;
    double originY = 0;
    user_to_device(&originX, &originY);
    user_to_device(dx, dy);
    (*dx) = (*dx) - originX;
    (*dy) = (*dy) - originY;
}

void OpenGLPainter::device_to_user_distance(double* dx, double* dy) {
    double originX = 0;
    double originY = 0;

    device_to_user(&originX, &originY);
    device_to_user(dx, dy);
    (*dx) = (*dx) - originX;
    (*dy) = (*dy) - originY;
}

void OpenGLPainter::new_path() {

}

void OpenGLPainter::close_path() {

}

void OpenGLPainter::new_sub_path() {

}

void OpenGLPainter::rotate(double r) {
    glRotated(r * 180.0f / M_PI, 0, 0, 1);
}

void OpenGLPainter::arc(double x, double y, double r, double start, double end) {

}

void OpenGLPainter::arcNegative(double x, double y, double r, double start, double end) {

}

void OpenGLPainter::circle(double x, double y, double r) {

}

void OpenGLPainter::ellipse(double cx, double cy, double rx, double ry, double sa, double ea, double ra) {

}

void OpenGLPainter::font_size(double size, bool deviceCoords) {

}

void OpenGLPainter::select_font_face(const char* text_val) {

}

void OpenGLPainter::text(const char* text_val) {

}

TextExtends OpenGLPainter::text_extends(const char* text_val) {

}

void OpenGLPainter::quadratic_curve_to(double x1, double y1, double x2, double y2) {

}

void OpenGLPainter::curve_to(double x1, double y1, double x2, double y2, double x3, double y3) {

}

long OpenGLPainter::pattern_create_linear(double x1, double y1, double x2, double y2) {

}

void OpenGLPainter::pattern_add_color_stop_rgba(long pat, double offset, double r, double g, double b, double a) {

}

void OpenGLPainter::set_pattern_source(long pat) {

}

void OpenGLPainter::pattern_destroy(long pat) {

}

void OpenGLPainter::point(double x, double y, double size, bool deviceCoords) {

}

void OpenGLPainter::reset_transformations() {
    glLoadIdentity();
}

unsigned char* OpenGLPainter::data() {
    return nullptr;
}

long OpenGLPainter::image_create(const std::string& file) {
    return 0;
}

void OpenGLPainter::image_destroy(long image) {

}

void OpenGLPainter::image(long image, double uvx, double vy, double vvx, double vvy, double x, double y) {

}
