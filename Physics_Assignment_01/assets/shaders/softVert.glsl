uniform mat4 mModel;
uniform mat4 mView;
uniform mat4 mProjection;

// "Vertex" attribute
attribute vec3 vCol;
attribute vec3 vPos;

out vec3 color;

void main()
{	
	vec3 position = vPos;
	
	// Calculate the model view projection matrix here
	mat4 MVP = mProjection * mView * mModel;
	
	//Multiply the vertex position by the transformation matrix
	gl_Position = MVP * vec4(position, 1.0f);
	
    color = vCol;
}