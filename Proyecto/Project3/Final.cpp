/*---------------------------------------------------------*/
/* ----------------  Proyecto Final -----------------------*/
/*-----------------    2022-2   ---------------------------*/

#include <Windows.h>

#include <glad/glad.h>
#include <glfw3.h>	//main
#include <stdlib.h>		
#include <glm/glm.hpp>	//camera y model
#include <glm/gtc/matrix_transform.hpp>	//camera y model
#include <glm/gtc/type_ptr.hpp>
#include <time.h>


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>	//Texture

#define SDL_MAIN_HANDLED
#include <SDL/SDL.h>

#include <shader_m.h>
#include <camera.h>
#include <modelAnim.h>
#include <model.h>
#include <Skybox.h>
#include <iostream>

//#pragma comment(lib, "winmm.lib")

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
//void my_input(GLFWwindow *window);
void my_input(GLFWwindow* window, int key, int scancode, int action, int mods);
void animate(void);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
GLFWmonitor *monitors;

void getResolution(void);

// camera
Camera camera(glm::vec3(0.0f, 10.0f, 90.0f));
float MovementSpeed = 0.1f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
const int FPS = 60;
const int LOOP_TIME = 1000 / FPS; // = 16 milisec // 1000 millisec == 1 sec
double	deltaTime = 0.0f,
		lastFrame = 0.0f;

//Lighting
glm::vec3 lightPosition(0.0f, 4.0f, -10.0f);
glm::vec3 lightDirection(0.0f, -1.0f, -1.0f);

//Keyframes (Manipulación y dibujo)
float	posX = 0.0f,
		posY = 0.0f,
		posZ = 0.0f,

		incX = 0.0f,
		incY = 0.0f,
		incZ = 0.0f;

//Este enum sirve para identificar en el arreglo de floats los parámetros del velociraptor;
enum RaptorParams
{
	RaptorPistaRadio,
	RaptorPistaAngulo,
	RaptorRotCuerpoY,
	RaptorRotCabezaX,
	RaptorRotCabezaY,
	RaptorRotMandibula,
	RaptorRotBrazos,
	RaptorRotPatas,
	RaptorRotColaX,
	RaptorRotColaY,
	RaptorMaxParams
};
float RaptorParam[RaptorMaxParams] = {	0.0f};
//Este arreglo sirve para poder alterar los valores de incrementos.
float RaptorParamInc[RaptorMaxParams] = { 0.0f };
#define MAX_FRAMES 9
int i_max_steps = 60;
int i_curr_steps = 0;
typedef struct _frame
{
	//Variables para GUARDAR Key Frames
	float posX;		//Variable para PosicionX
	float posY;		//Variable para PosicionY
	float posZ;		//Variable para PosicionZ
	float rotRodIzq;
	float giroMonito;
	float RaptorKFParams[RaptorMaxParams];

}FRAME;

FRAME KeyFrame[MAX_FRAMES];

int FrameIndex = 3;			//introducir datos
bool play = false;
int playIndex = 0;

void saveFrame(void)
{
	//printf("frameindex %d\n", FrameIndex);
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	KeyFrame[FrameIndex].posX = posX;
	KeyFrame[FrameIndex].posY = posY;
	KeyFrame[FrameIndex].posZ = posZ;
	for (size_t i = 0; i < RaptorMaxParams; i++)
	{
		KeyFrame[FrameIndex].RaptorKFParams[i] = RaptorParam[i];
		std::cout << " RP[" << i<<"] "<< RaptorParam[i];
	}
	std::cout << std::endl;
	FrameIndex++;
}

void resetElements(void)
{
	posX = KeyFrame[0].posX;
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;

	for (size_t i = 2; i < RaptorMaxParams; i++)
	{
		RaptorParam[i] = KeyFrame[0].RaptorKFParams[i];

	}
}

void interpolation(void)
{
	incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
	incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;
	for (size_t i = 0; i < RaptorMaxParams; i++)
	{
		RaptorParamInc[i] = (KeyFrame[playIndex + 1].RaptorKFParams[i] - KeyFrame[playIndex].RaptorKFParams[i]) / i_max_steps;

	}
}

void animate(void)
{
	if (play)
	{
		if (i_curr_steps >= i_max_steps) //end of animation between frames?
		{
			playIndex++;
			if (playIndex > FrameIndex - 2)	//end of total animation?
			{
				std::cout << "Animation ended" << std::endl;
				//printf("termina anim\n");
				playIndex = 0;
				resetElements();
				//play = false;
			}
			i_curr_steps = 0; //Reset counter
							  //Interpolation
			interpolation();
			
		}
		else
		{
			//Draw animation
			posX += incX;
			posY += incY;
			posZ += incZ;
			RaptorParam[RaptorPistaAngulo]+=0.1;
			for (size_t i = 0; i < RaptorMaxParams; i++)
			{
				RaptorParam[i] += RaptorParamInc[i];
			}


			i_curr_steps++;
		}
	}
}

void getResolution()
{
	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

	SCR_WIDTH = mode->width;
	SCR_HEIGHT = (mode->height) - 80;
}


int main()
{
	KeyFrame[0].RaptorKFParams[0] = 0.0f;
	KeyFrame[0].RaptorKFParams[1] = 0.0f;
	KeyFrame[0].RaptorKFParams[2] = -15.0f;
	KeyFrame[0].RaptorKFParams[3] = 0.0f;
	KeyFrame[0].RaptorKFParams[4] = 18.0f;
	KeyFrame[0].RaptorKFParams[5] = -24.0f;
	KeyFrame[0].RaptorKFParams[6] = -38.0f;
	KeyFrame[0].RaptorKFParams[7] = -48.0f;
	KeyFrame[0].RaptorKFParams[8] = 0.0f;
	KeyFrame[0].RaptorKFParams[9] = 30.0f;
	//{ 0.0f, 0.0f, 15.0f, 0.0f, 18.0f, -24.0f, - 38.0f, -48.0f, 0.0f , 30.0f };
	KeyFrame[1].RaptorKFParams[0] = 0.0f;
	KeyFrame[1].RaptorKFParams[1] = 0.0f;
	KeyFrame[1].RaptorKFParams[2] = 12.0f;
	KeyFrame[1].RaptorKFParams[3] = 0.0f;
	KeyFrame[1].RaptorKFParams[4] = -4.0f;
	KeyFrame[1].RaptorKFParams[5] = -6.0f;
	KeyFrame[1].RaptorKFParams[6] = 42.0f;
	KeyFrame[1].RaptorKFParams[7] = 49.0f;
	KeyFrame[1].RaptorKFParams[8] = 0.0f;
	KeyFrame[1].RaptorKFParams[9] = -12.0f;
	//{ 0.0f, 0.0f, 15.0f, 0.0f, 18.0f, -24.0f, - 38.0f, -48.0f, 0.0f , 30.0f };
	KeyFrame[2].RaptorKFParams[0] = 0.0f;
	KeyFrame[2].RaptorKFParams[1] = 0.0f;
	KeyFrame[2].RaptorKFParams[2] = -15.0f;
	KeyFrame[2].RaptorKFParams[3] = 0.0f;
	KeyFrame[2].RaptorKFParams[4] = 18.0f;
	KeyFrame[2].RaptorKFParams[5] = -24.0f;
	KeyFrame[2].RaptorKFParams[6] = -38.0f;
	KeyFrame[2].RaptorKFParams[7] = -48.0f;
	KeyFrame[2].RaptorKFParams[8] = 0.0f;
	KeyFrame[2].RaptorKFParams[9] = 30.0f;
	//{ 0.0f, 0.0f, 15.0f, 0.0f, 18.0f, -24.0f, - 38.0f, -48.0f, 0.0f , 30.0f };
	KeyFrame[3].RaptorKFParams[0] = 0.0f;
	KeyFrame[3].RaptorKFParams[1] = 0.0f;
	KeyFrame[3].RaptorKFParams[2] = 12.0f;
	KeyFrame[3].RaptorKFParams[3] = 0.0f;
	KeyFrame[3].RaptorKFParams[4] = -4.0f;
	KeyFrame[3].RaptorKFParams[5] = -6.0f;
	KeyFrame[3].RaptorKFParams[6] = 42.0f;
	KeyFrame[3].RaptorKFParams[7] = 49.0f;
	KeyFrame[3].RaptorKFParams[8] = 0.0f;
	KeyFrame[3].RaptorKFParams[9] = -12.0f;
	//{ 0.0f, 0.0f, 15.0f, 0.0f, 18.0f, -24.0f, - 38.0f, -48.0f, 0.0f , 30.0f };
	
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	// --------------------
	monitors = glfwGetPrimaryMonitor();
	getResolution();

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "CGeIHC", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowPos(window, 0, 30);
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetKeyCallback(window, my_input);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader staticShader("Shaders/shader_Lights.vs", "Shaders/shader_Lights.fs");
	Shader skyboxShader("Shaders/skybox.vs", "Shaders/skybox.fs");
	Shader animShader("Shaders/anim.vs", "Shaders/anim.fs");

	vector<std::string> faces
	{
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	Skybox skybox = Skybox(faces);

	// Shader configuration
	// --------------------
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);

	// load models
	// -----------
	//Model piso("resources/objects/piso/piso.obj");
	Model isla("resources/objects/Isla/isla.obj");
	Model cubo("resources/objects/cubo.obj");
	Model Silla("resources/objects/Silla/old_table.obj");
	Model Raptor("resources/ObjectsRodrigo/Raptor/Raptor.obj");
	Model Roca("resources/ObjectsRodrigo/Arboles/LowPolyTree.obj");
	Model Edificio1("resources/ObjectsRodrigo/Edificios/Edif1.obj");
	Model Curva("resources/ObjectsRodrigo/Caminos/Curva.obj");
	
	
	Model RaptorCuerpo("resources/ObjectsRodrigo/Raptor/Cuerpo.obj");
	Model RaptorCola("resources/ObjectsRodrigo/Raptor/Cola.obj");
	Model RaptorCabeza("resources/ObjectsRodrigo/Raptor/Cabeza.obj");
	Model RaptorMandibula("resources/ObjectsRodrigo/Raptor/Mandibula.obj");
	Model RaptorBrazoIzq("resources/ObjectsRodrigo/Raptor/BrazoIzq.obj");
	Model RaptorBrazoDer("resources/ObjectsRodrigo/Raptor/BrazoDer.obj");
	Model RaptorPataIzq("resources/ObjectsRodrigo/Raptor/PataIzq.obj");
	Model RaptorPataDer("resources/ObjectsRodrigo/Raptor/PataDer.obj");
	
	//Model Estatua("resources/objects/DinoParque/Dinosaurio/Stegosaurus.obj");
	
	//Inicialización de KeyFrames
	for (int i = 0; i < MAX_FRAMES; i++)
	{
		KeyFrame[i].posX = 0;
		KeyFrame[i].posY = 0;
		KeyFrame[i].posZ = 0;
		KeyFrame[i].rotRodIzq = 0;
		KeyFrame[i].giroMonito = 0;
	}

	while (!glfwWindowShouldClose(window))
	{
		skyboxShader.setInt("skybox", 0);
		
		// per-frame time logic
		// --------------------
		lastFrame = SDL_GetTicks();

		// input
		// -----
		//my_input(window);
		animate();

		// render
		// ------
		glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// don't forget to enable shader before setting uniforms
		staticShader.use();
		//Setup Advanced Lights
		staticShader.setVec3("viewPos", camera.Position);
		staticShader.setVec3("dirLight.direction", lightDirection);
		staticShader.setVec3("dirLight.ambient", glm::vec3(1.0f, 1.0f, 1.0f));
		staticShader.setVec3("dirLight.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("dirLight.specular", glm::vec3(0.0f, 0.0f, 0.0f));

		staticShader.setVec3("pointLight[0].position", lightPosition);
		staticShader.setVec3("pointLight[0].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[0].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[0].constant", 0.08f);
		staticShader.setFloat("pointLight[0].linear", 0.009f);
		staticShader.setFloat("pointLight[0].quadratic", 0.032f);

		staticShader.setVec3("pointLight[1].position", glm::vec3(-80.0, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setVec3("pointLight[1].specular", glm::vec3(0.0f, 0.0f, 0.0f));
		staticShader.setFloat("pointLight[1].constant", 1.0f);
		staticShader.setFloat("pointLight[1].linear", 0.009f);
		staticShader.setFloat("pointLight[1].quadratic", 0.032f);

		staticShader.setFloat("material_shininess", 32.0f);

		glm::mat4 model = glm::mat4(1.0f);
		glm::mat4 tmp = glm::mat4(1.0f);
		// view/projection transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 10000.0f);
		glm::mat4 view = camera.GetViewMatrix();
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		//// Light
		glm::vec3 lightColor = glm::vec3(0.6f);
		glm::vec3 diffuseColor = lightColor * glm::vec3(0.5f);
		glm::vec3 ambientColor = diffuseColor * glm::vec3(0.75f);
		

		// -------------------------------------------------------------------------------------------------------------------------
		// Personaje Animacion
		// -------------------------------------------------------------------------------------------------------------------------
		//Remember to activate the shader with the animation
		animShader.use();
		animShader.setMat4("projection", projection);
		animShader.setMat4("view", view);
	
		animShader.setVec3("material.specular", glm::vec3(0.5f));
		animShader.setFloat("material.shininess", 32.0f);
		animShader.setVec3("light.ambient", ambientColor);
		animShader.setVec3("light.diffuse", diffuseColor);
		animShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		animShader.setVec3("light.direction", lightDirection);
		animShader.setVec3("viewPos", camera.Position);

		
		// -------------------------------------------------------------------------------------------------------------------------
		// Escenario
		// -------------------------------------------------------------------------------------------------------------------------
		staticShader.use();//Dibujar las cosas después de este shader
		staticShader.setMat4("projection", projection);
		staticShader.setMat4("view", view);

		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(20.0f));
		staticShader.setMat4("model", model);
		isla.Draw(staticShader);

		//DinoParque
		/*
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-250.0f, 0.0f, -400.0f));//Colocando Silla
		model = glm::scale(model, glm::vec3(3.5f));
		staticShader.setMat4("model", model);
		Silla.Draw(staticShader);
		*/
		model = glm::translate(glm::mat4(1.0f), glm::vec3(10.00f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		cubo.Draw(staticShader);
		
		model = glm::translate(glm::mat4(1.0f), glm::vec3(00.00f, 10.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		cubo.Draw(staticShader);


		/*----------------------------------------------------------------------------------
		------------------------------------------------------------------------------------
		---------CREANDO PISTA DE CARRERAS DE VELOCIRAPTORS---------------------------------
		------------------------------------------------------------------------------------
		----------------------------------------------------------------------------------*/
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.00f, 0.1f, 10.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		Curva.Draw(staticShader);
		
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorPistaAngulo]), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::translate(model, glm::vec3(-64.5f+RaptorParam[RaptorPistaRadio], 0.2f, 0.0f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotCuerpoY]),glm::vec3(0.0f,1.0f,0.0f));
		tmp = model;		
		staticShader.setMat4("model", model);
		RaptorCuerpo.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(0.00f, 1.42f, 0.640f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotCabezaX]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotCabezaY]), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		RaptorCabeza.Draw(staticShader);

		model = glm::translate(model, glm::vec3(0.00f, 00.0f,0.190f ));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotMandibula]+30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		RaptorMandibula.Draw(staticShader);


		model = glm::translate(tmp, glm::vec3(0.050f, 1.0f, 0.50f));
		model = glm::rotate(model, glm::radians(-RaptorParam[RaptorRotBrazos]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		RaptorBrazoIzq.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(-0.05f, 1.0f , 0.5f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotBrazos]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		RaptorBrazoDer.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(0.065f, 1.0f, 0.160f));
		model = glm::rotate(model, glm::radians(-RaptorParam[RaptorRotPatas]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(1.0f));
		staticShader.setMat4("model", model);
		RaptorPataIzq.Draw(staticShader);

		model = glm::translate(tmp, glm::vec3(-0.065f,  1.0f, 0.160f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotPatas]), glm::vec3(1.0f, 0.0f, 0.0f));
		staticShader.setMat4("model", model);
		RaptorPataDer.Draw(staticShader);


		model = glm::translate(tmp, glm::vec3(0.00f, 1.0f, -0.420f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotColaX]), glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, glm::radians(RaptorParam[RaptorRotColaY]), glm::vec3(0.0f, 1.0f, 0.0f));
		staticShader.setMat4("model", model);
		RaptorCola.Draw(staticShader);
		/*
		model = glm::translate(glm::mat4(1.0f), glm::vec3(-150.0f, 0.0f, -100.0f));//Colocando Dinosaurio
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		Estatua.Draw(staticShader);
		*/
		
		// -------------------------------------------------------------------------------------------------------------------------
		// Caja Transparente --- Siguiente Práctica
		// -------------------------------------------------------------------------------------------------------------------------
		/*glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -70.0f));
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		cubo.Draw(staticShader);
		glEnable(GL_BLEND);*/
		// -------------------------------------------------------------------------------------------------------------------------
		// Termina Escenario
		// -------------------------------------------------------------------------------------------------------------------------

		//-------------------------------------------------------------------------------------
		// draw skybox as last
		// -------------------
		skyboxShader.use();
		skybox.Draw(skyboxShader, view, projection, camera);

		// Limitar el framerate a 60
		deltaTime = SDL_GetTicks() - lastFrame; // time for full 1 loop
		//std::cout <<"frame time = " << frameTime << " milli sec"<< std::endl;
		if (deltaTime < LOOP_TIME)
		{
			SDL_Delay((int)(LOOP_TIME - deltaTime));
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	skybox.Terminate();

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void my_input(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, (float)deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, (float)deltaTime);
	//To Configure Model
	if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
		RaptorParam[RaptorRotCuerpoY]++;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		RaptorParam[RaptorRotCuerpoY]--;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		RaptorParam[RaptorRotCabezaX]++;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		RaptorParam[RaptorRotCabezaX]--;
	if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
		RaptorParam[RaptorRotCabezaY]++;
	if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
		RaptorParam[RaptorRotCabezaY]--;
	if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
		RaptorParam[RaptorRotMandibula]++;
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS)
		RaptorParam[RaptorRotMandibula]--;
	if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
		RaptorParam[RaptorRotBrazos]++;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		RaptorParam[RaptorRotBrazos]--;
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
		RaptorParam[RaptorRotPatas]++;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		RaptorParam[RaptorRotPatas]--;
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
		RaptorParam[RaptorRotColaY]--;
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		RaptorParam[RaptorRotColaY]++;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		RaptorParam[RaptorRotColaX]++;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		RaptorParam[RaptorRotColaX]--;

	//To play KeyFrame animation 
	if (key == GLFW_KEY_P && action == GLFW_PRESS)
	{
		if (play == false && (FrameIndex > 1))
		{
			std::cout << "Play animation" << std::endl;
			resetElements();
			//First Interpolation				
			interpolation();

			play = true;
			playIndex = 0;
			i_curr_steps = 0;
		}
		else
		{
			play = false;
			std::cout << "Not enough Key Frames" << std::endl;
		}
	}

	//To Save a KeyFrame
	if (key == GLFW_KEY_L && action == GLFW_PRESS)
	{
		if (FrameIndex < MAX_FRAMES)
		{
			saveFrame();
		}
	}
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}
// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}