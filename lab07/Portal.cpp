#include "Portal.h"
#include <glm/gtc/matrix_transform.hpp>
#include <common/model.h>
#include <iostream>
#include "common/texture.h"


using namespace glm;

int Portal::num_of_portals_spawned = 0;

Portal::Portal(GLFWwindow* window) { 

	portal = new Drawable(vertices,uvs,normals);
	//creating bounding box from cube
	box = new Drawable("models/cube.obj"); //xreiazomai auta ta vertices gia ta AABBs
	modelMatrix = mat4(1.0f);
	calculatePortalAABBs(); 
	
	if (num_of_portals_spawned == 0) texture = loadBMP("textures/bluehair.bmp"); //gia otan den ehume 2 portals spawned
	else if (num_of_portals_spawned == 1) texture = loadBMP("textures/greenhair.bmp");
	num_of_portals_spawned++;
}

void Portal::calculatePortalAABBs(mat4 modelMatrix) {

	std::vector<vec3> modelVertices = box->indexedVertices; //vertices se modelCoordinates
	
	this->frontBoundingBox = new AABB; //arxikopoiw to bounding box (pithano memory leak?)
	this->backBoundingBox = new AABB;
	for (int i = 0; i < modelVertices.size(); i++)
	{
		//metavainw se world coordinates
		//kanw ena anomoiomorfo scale gia na einai pio lepto to box kai na efarmozei kalitera sto portal
		//metakinw mprosta ston z efoson to portal einai strameno pros to +z prin pollaplasiasw me to portal modelmatrix
		vec4 frontWorldVertex = modelMatrix * translate(mat4(), vec3(0, 0, portalBoxDistance))* scale(mat4(), vec3( 1.3f, 3.0f, 1.5f))*  
			vec4(modelVertices[i].x, modelVertices[i].y, modelVertices[i].z, 1.0);

		float x = frontWorldVertex.x;
		float y = frontWorldVertex.y;
		float z = frontWorldVertex.z;

		if (this->frontBoundingBox->min_x > x) this->frontBoundingBox->min_x = x;
		if (this->frontBoundingBox->max_x < x) this->frontBoundingBox->max_x = x;

		if (this->frontBoundingBox->min_y > y) this->frontBoundingBox->min_y = y;
		if (this->frontBoundingBox->max_y < y) this->frontBoundingBox->max_y = y;

		if (this->frontBoundingBox->min_z > z) this->frontBoundingBox->min_z = z;
		if (this->frontBoundingBox->max_z < z) this->frontBoundingBox->max_z = z;

		vec4 backWorldVertex = modelMatrix * translate(mat4(), vec3(0,0, -portalBoxDistance))* scale(mat4(), vec3(1.3f, 3.0f, 1.5f))* 
			vec4(modelVertices[i].x, modelVertices[i].y, modelVertices[i].z, 1.0);


		x = backWorldVertex.x;
		y = backWorldVertex.y;
		z = backWorldVertex.z;

		if (this->backBoundingBox->min_x > x) this->backBoundingBox->min_x = x;
		if (this->backBoundingBox->max_x < x) this->backBoundingBox->max_x = x;

		if (this->backBoundingBox->min_y > y) this->backBoundingBox->min_y = y;
		if (this->backBoundingBox->max_y < y) this->backBoundingBox->max_y = y;

		if (this->backBoundingBox->min_z > z) this->backBoundingBox->min_z = z;
		if (this->backBoundingBox->max_z < z) this->backBoundingBox->max_z = z;

	}
	//den xreiazomaste center kai extents opws sto playerAABB
	//extra:tha mporousame na ftiaksoume ta portals na topothetountai kalitera (na mhn temnoun katheta kapoion toixo giati to apotelesma pou paragetai einai mh realistiko)
	//me plane-AABB collision
	//an apagoreuoume to spawn an to portalAABB temnetai apo kapoio wall
}

void Portal::drawBoundingBox(GLuint modelMatrixLocation) { //zwgrafizetai ksexwrista an pote xreiastei na doume to kouti

	Drawable* boxOutline = new Drawable("models/cube.obj");

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_CULL_FACE);
	boxOutline->bind();
	mat4 frontModelMatrix =  modelMatrix *translate(mat4(),vec3(0,0, portalBoxDistance))* scale(mat4(), vec3(1.3f, 3.0f, 1.5f));
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &frontModelMatrix[0][0]);
	boxOutline->draw();
	mat4 backModelMatrix = modelMatrix * translate(mat4(), vec3(0, 0, -portalBoxDistance)) * scale(mat4(), vec3(1.3f, 3.0f, 1.5f));
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &backModelMatrix[0][0]);
	boxOutline->draw();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_CULL_FACE);
}

void Portal::drawOutline(GLuint modelMatrixLocation,GLuint textureSamplerLocation) {
	//gia na ksexwrizoume ta portals
	portal->bind();
	vec3 normal = vec3(portalNormal.x, portalNormal.y, portalNormal.z);
	mat4 scaledModel = translate(mat4(),-0.005f * normal)*modelMatrix*scale(mat4(), vec3(1.05f, 1.05f, 1.05f)); 
	//translate ligo pio pisw gia na min ehume miksi me to portal image kai scale ligo pio megalo gia na exoume perigramma
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &scaledModel[0][0]);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D,texture);
	glUniform1i(textureSamplerLocation, 2);
	portal->draw();
}

void Portal::draw(GLuint modelMatrixLocation)
{
	portal->bind();
	glUniformMatrix4fv(modelMatrixLocation, 1, GL_FALSE, &this->modelMatrix[0][0]);
	portal->draw();
}

void Portal::erasePortal() {
	delete this;
	num_of_portals_spawned--;
}

void Portal::spawnPortal(Portal& portal, vec3 wallPosition, vec3 wallNormal, GLFWwindow* window) //spawn panw se wall
{
	mat4 rotationMatrix;
	mat4 translationMatrix=translate(mat4(),wallPosition);

	float theta = 3.14f / 2.0f;
	vec3 xAxis = vec3(1.0, 0.0, 0.0);
	vec3 yAxis = vec3(0.0, 1.0, 0.0);
	vec3 zAxis = vec3(0.0, 0.0, 1.0);

	if (wallNormal == -yAxis) rotationMatrix = rotate(mat4(), -theta, yAxis)*  rotate(mat4(), theta, xAxis);
	else if (wallNormal == yAxis) rotationMatrix = rotate(mat4(), -theta, yAxis)*rotate(mat4(), -theta, xAxis);

	else if (wallNormal == zAxis) rotationMatrix = mat4(1.0f);
	else if (wallNormal == -zAxis) rotationMatrix = rotate(mat4(), 2 * theta, yAxis);

	else if (wallNormal == -xAxis) rotationMatrix = rotate(mat4(), -theta, yAxis);
	else if(wallNormal==xAxis) rotationMatrix = rotate(mat4(), theta, yAxis);

	portal.modelMatrix = translationMatrix * rotationMatrix;
	portal.portalNormal = vec4(wallNormal, 0.0f); 
	portal.portalUpVector = rotationMatrix *vec4(0.0f,1.0f,0.0f,0.0f);
	portal.position = translationMatrix * vec4(0.0f,0.0f,0.0f,1.0f); //neo position sto neo spawn apo neo modelMatrix
	
	portal.calculatePortalAABBs(portal.modelMatrix); //epanaipologizw ta AABBs me ton neo modelMatrix

	return;
} 
