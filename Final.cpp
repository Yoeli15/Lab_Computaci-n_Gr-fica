/*---------------------------------------------------------*/
/* ----------------  Proyecto Final -----------------------*/
/*-----------------    2022-2   ---------------------------*/
/*-------------------Integrantes:---------------------------------*/
/*-----------Cortés González Zaira Yaritsi------------------------*/

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
		incZ = 0.0f,
		
		rot1 = 0.0f,
		rot2 = -90.0f;

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

}FRAME;

FRAME KeyFrame[MAX_FRAMES];
int FrameIndex = 0;			//introducir datos
bool play = false;
int playIndex = 0;

void saveFrame(void)
{
	//printf("frameindex %d\n", FrameIndex);
	std::cout << "Frame Index = " << FrameIndex << std::endl;

	KeyFrame[FrameIndex].posX = posX;
	KeyFrame[FrameIndex].posY = posY;
	KeyFrame[FrameIndex].posZ = posZ;

	FrameIndex++;
}

void resetElements(void)
{
	posX = KeyFrame[0].posX;
	posY = KeyFrame[0].posY;
	posZ = KeyFrame[0].posZ;
}

void interpolation(void)
{
	incX = (KeyFrame[playIndex + 1].posX - KeyFrame[playIndex].posX) / i_max_steps;
	incY = (KeyFrame[playIndex + 1].posY - KeyFrame[playIndex].posY) / i_max_steps;
	incZ = (KeyFrame[playIndex + 1].posZ - KeyFrame[playIndex].posZ) / i_max_steps;
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
				play = false;
			}
			else //Next frame interpolations
			{
				i_curr_steps = 0; //Reset counter
								  //Interpolation
				interpolation();
			}
		}
		else
		{
			//Draw animation
			posX += incX;
			posY += incY;
			posZ += incZ;

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
	Model Banco1("resources/objects/Bancos/Banco1/old_table.obj");
	Model Banco4("resources/objects/Bancos/Banco4/Banco4.obj");
	Model Arbol1("resources/objects/Arboles/Arbol1.obj");
	Model Arbol2("resources/objects/Arboles/Arbol2.obj");
	Model Cerca("resources/objects/Cerca/Cerca.obj");
	Model Entrada("resources/objects/Entrada/Entrada.obj");
	Model Lampara("resources/objects/Lamparas/Lampara.obj");
	Model Kiosko("resources/objects/Kiosko/Prueba.obj");
	Model Mesa("resources/objects/Mesa/Mesa.obj");
	Model Resbaladilla("resources/objects/Resbaladilla/Resbaladilla.obj");
	Model Dinosaurio("resources/objects/Dinosaurios/Triceratop/TriceratopMejora.obj");
	Model Helados("resources/objects/CarroHelados/carrito_helado.obj");
	Model Basura("resources/objects/BotesBasura/Basura.obj");
	Model BasuraIn("resources/objects/BotesBasura/BasuraIn.obj");
	Model Juego("resources/objects/Juego/Teeter03.obj");
	Model Maquina("resources/objects/Maquinas/Maquina.obj");
	Model Pasamanos("resources/objects/Pasamanos/Prueba.obj");
	Model SubeBaja("resources/objects/SubeBaja/SubeBaja.obj");
	Model Casa1("resources/objects/Casas/Casa1/3d-model.obj");
	Model CasaF("resources/objects/Casas/Casaf/3d-model.obj");
	Model CasaAzul("resources/objects/Casas/CasaAzul/3d-model.obj");
	
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

		//DINOPARQUE

		model = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, -0.5f, -1170.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(475.0f, -0.5f, -1100.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(450.0f, -0.5f, -1030.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(423.0f, 0.0f, -967.5f));//Colocando Entrada
		model = glm::rotate(model, glm::radians(rot1+60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.07f));
		staticShader.setMat4("model", model);
		Entrada.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(350.0f, -0.7f, -880.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(410.0f, -0.7f, -1070.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(490.0f, -1.0f, -930.0f));//Colocando Dinosaurio
		model = glm::rotate(model, glm::radians(rot1-20), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		Dinosaurio.Draw(staticShader);
		
		model = glm::translate(glm::mat4(1.0f), glm::vec3(396.0f, -0.5f, -902.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(371.0f, -0.5f, -832.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(346.0f, -0.5f, -762.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(321.0f, -0.5f, -692.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 70.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, -0.5f, -1000.0f));//Colocando Resbaladilla
		model = glm::rotate(model, glm::radians(rot1 + 50), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Resbaladilla.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(550.0f, 3.3f, -1060.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f));
		staticShader.setMat4("model", model);
		Banco1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(600.0f, -1.0f, -1060.0f));//Colocando BasuraOrg
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		Basura.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, 3.3f, -1060.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot2), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f));
		staticShader.setMat4("model", model);
		Banco1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(620.0f, -1.0f, -950.0f));//Colocando Mesapicnic
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Mesa.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, -1050.0f));//Colocando Arbol
		model = glm::scale(model, glm::vec3(0.02f));
		staticShader.setMat4("model", model);
		Arbol1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, -0.7f, -950.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(720.0f, -1.0f, -1000.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot2), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.12f));
		staticShader.setMat4("model", model);
		Banco4.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(720.0f, -1.0f, -850.0f));//Colocando Maquina
		model = glm::rotate(model, glm::radians(rot1+180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(13.0f));
		staticShader.setMat4("model", model);
		Maquina.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, -760.0f));//Colocando Arbol
		model = glm::scale(model, glm::vec3(0.02f));
		staticShader.setMat4("model", model);
		Arbol2.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, -1.0f, -750.0f));//Colocando Mesapicnic
		model = glm::rotate(model, glm::radians(rot1), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Mesa.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(680.0f, -1.0f, -870.0f));//Colocando Sube y baja
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		SubeBaja.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -1015.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -941.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -867.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -643.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -569.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(750.0f, -0.5f, -495.0f));//Colocando Cerca
		model = glm::rotate(model, glm::radians(rot1 + 90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(5.5f));
		staticShader.setMat4("model", model);
		Cerca.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, -0.7f, -590.0f));//Colocando Lámpara
		model = glm::scale(model, glm::vec3(5.0f));
		staticShader.setMat4("model", model);
		Lampara.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 0.0f, -650.0f));//Colocando Pasamanos
		model = glm::scale(model, glm::vec3(15.0f));
		staticShader.setMat4("model", model);
		Pasamanos.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(700.0f, 3.0f, -540.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(3.0f));
		staticShader.setMat4("model", model);
		Banco1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, -1.0f, -540.0f));//Colocando Maquina
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(13.0f));
		staticShader.setMat4("model", model);
		Maquina.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(600.0f, -1.0f, -540.0f));//Colocando BasuraIn
		model = glm::rotate(model, glm::radians(rot1 + 180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		BasuraIn.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, -1.0f, -540.0f));//Colocando Silla
		model = glm::rotate(model, glm::radians(rot1+180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.12f));
		staticShader.setMat4("model", model);
		Banco4.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(500.0f, 7.0f, -680.0f));//Colocando Juego
		model = glm::scale(model, glm::vec3(0.03f));
		staticShader.setMat4("model", model);
		Juego.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(560.0f, -1.0f, -900.0f));//Colocando BasuraOrg
		model = glm::rotate(model, glm::radians(rot1 - 90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		Basura.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(580.0f, -1.0f, -840.0f));//Colocando Kiosko
		model = glm::rotate(model, glm::radians(rot1 + 180), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Kiosko.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(520.0f, -1.0f, -800.0f));//Colocando BasuraIn
		model = glm::rotate(model, glm::radians(rot1 - 100), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(4.5f));
		staticShader.setMat4("model", model);
		BasuraIn.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(620.0f, -1.0f, -630.0f));//Colocando Mesapicnic
		model = glm::rotate(model, glm::radians(rot1+90), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Mesa.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(600.0f, -1.0f, -700.0f));//Colocando Sube y baja
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		SubeBaja.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(430.0f, -0.5f, -800.0f));//Colocando Resbaladilla
		model = glm::rotate(model, glm::radians(rot1+70), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.15f));
		staticShader.setMat4("model", model);
		Resbaladilla.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(400.0f, 0.0f, -700.0f));//Colocando Carrito
		model = glm::scale(model, glm::vec3(0.5f));
		staticShader.setMat4("model", model);
		Helados.Draw(staticShader);
		
		//ZONA RESIDENCIAL
		model = glm::translate(glm::mat4(1.0f), glm::vec3(850.0f, 0.0f, -800.0f));//Colocando 
		model = glm::rotate(model, glm::radians(rot1 - 60), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f));
		staticShader.setMat4("model", model);
		Casa1.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(150.0f, 0.0f, -967.5f));//Colocando 
		model = glm::rotate(model, glm::radians(rot1 + 60.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.4f));
		staticShader.setMat4("model", model);
		CasaF.Draw(staticShader);

		model = glm::translate(glm::mat4(1.0f), glm::vec3(650.0f, 0.0f, -300.0f));
		model = glm::rotate(model, glm::radians(rot1), glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::scale(model, glm::vec3(0.1f));
		staticShader.setMat4("model", model);
		CasaAzul.Draw(staticShader);
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
		posZ++;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
		posZ--;
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
		posX--;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		posX++;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		lightPosition.x++;
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		lightPosition.x--;

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