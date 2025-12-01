#include "function.h"
#include <random>

// --- [네트워크] 1. GameClient 헤더 및 외부 변수 선언 ---
#include "GameClient.h"

extern GameClient* g_NetworkClient;

// [중요] main.cpp에서 접속 성공 후, 이 변수에 내 ID(0 또는 1)를 저장해야 합니다.
// 0이면 Player1(서버장), 1이면 Player2(참가자)로 동작합니다.
extern int g_PlayerID;


extern GLfloat Card[] = {
	-CARDSIZE, CARDSIZE, 0, 0, 1,
	CARDSIZE, CARDSIZE, 0, 1, 1,
	-CARDSIZE, -CARDSIZE, 0, 0, 0,
	CARDSIZE, CARDSIZE, 0, 1, 1,
	-CARDSIZE, -CARDSIZE, 0, 0, 0,
	CARDSIZE, -CARDSIZE, 0, 1, 0,
};

extern GLfloat cubeDots[][3] = {
	// 육면체
	{-BOXSIZE, BOXSIZE, BOXSIZE},
	{-BOXSIZE, -BOXSIZE, BOXSIZE},
	{BOXSIZE, -BOXSIZE, BOXSIZE},
	{BOXSIZE, BOXSIZE, BOXSIZE},
	{-BOXSIZE, BOXSIZE, -BOXSIZE},
	{-BOXSIZE, -BOXSIZE, -BOXSIZE},
	{BOXSIZE, -BOXSIZE, -BOXSIZE},
	{BOXSIZE, BOXSIZE, -BOXSIZE}
};

extern GLfloat textureST[][2] = {
	{0.0, 0.0},
	{1.0, 0.0},
	{0.0, 1.0},
	{1.0, 1.0},
	{0.5, 1.0}
};

extern unsigned int cubeShapeindex[] = {
	// 육면체
	0, 1, 2,
	0, 2, 3,
	7, 6, 5,
	7, 5, 4,
	3, 2, 6,
	3, 6, 7,
	4, 5, 1,
	4, 1, 0,
	4, 0, 3,
	4, 3, 7,
	1, 5, 6,
	1, 6, 2
};

extern unsigned int textureindex[] = {
	// 육면체
	2, 0, 1,
	2, 1, 3
};

extern unsigned int cubeNormalindex[] = {
	// 육면체
	0, 0, 0,
	0, 0, 0,
	1, 1, 1,
	1, 1, 1,
	2, 2, 2,
	2, 2, 2,
	3, 3, 3,
	3, 3, 3,
	4, 4, 4,
	4, 4, 4,
	5, 5, 5,
	5, 5, 5
};

extern GLfloat cubeNormal[][3] = {
	{0.0, 0.0, 1.0},
	{0.0, 0.0, -1.0},
	{1.0, 0.0, 0.0},
	{-1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, -1.0, 0.0}
};

extern GLuint g_window_w = 1600;
extern GLuint g_window_h = 900;
extern GLfloat cameraXYZ[3] = { 5.0f, 6.0f, 8.0f };

extern bool cameraRotateY = false, cameraRotateMinus = false;
extern bool MoveTime = false;

extern int currentPlayer = 0;
//extern int turn = 0;
extern int Player1Score = 0;
extern int Player2Score = 0;
extern int tmpScore = 0;

extern objectManager Player1[PLAYERNUM]{};
extern objectManager Player2[PLAYERNUM]{};

extern GLuint ShapeVAO{};
extern GLuint ShapeVBO{};

extern GLuint ObjVAO{};
extern GLuint ObjVBO{};
extern GLuint ObjTexVBO{};

extern GLuint CardVAO{};
extern GLuint CardVBO{};

extern CardManager Card1{};
extern CardManager Card2{};

extern unsigned int textures[16]{};
extern unsigned int objtextures[6]{};

extern float shapevertices[216]{};
extern float texturevertices[72]{};
extern objectManager Map[mapSize][mapSize]{};


void InitBuffer()
{
	glGenVertexArrays(1, &ShapeVAO);
	glGenBuffers(1, &ShapeVBO);

	glBindVertexArray(ShapeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, ShapeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shapevertices), shapevertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glEnable(GL_DEPTH_TEST);
}

void InitBuffer_obj()
{
	glGenVertexArrays(1, &ObjVAO);
	glGenBuffers(1, &ObjVBO);
	glGenBuffers(1, &ObjTexVBO);

	glBindVertexArray(ObjVAO);

	glBindBuffer(GL_ARRAY_BUFFER, ObjVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shapevertices), shapevertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, ObjTexVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(texturevertices), texturevertices, GL_STATIC_DRAW);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
}

void InitTexture_obj()
{
	int widthImage, heightImage, numberOfChannel;

	glGenTextures(1, &objtextures[0]);
	glBindTexture(GL_TEXTURE_2D, objtextures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("Player1_A.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &objtextures[1]);
	glBindTexture(GL_TEXTURE_2D, objtextures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Player1_S.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &objtextures[2]);
	glBindTexture(GL_TEXTURE_2D, objtextures[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Player1_D.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &objtextures[3]);
	glBindTexture(GL_TEXTURE_2D, objtextures[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Player2_A.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &objtextures[4]);
	glBindTexture(GL_TEXTURE_2D, objtextures[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Player2_S.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &objtextures[5]);
	glBindTexture(GL_TEXTURE_2D, objtextures[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Player2_D.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glUseProgram(s_Objprogram);
	int tLocation = glGetUniformLocation(s_Objprogram, "outTexture");
	glUniform1i(tLocation, 0);
}

void InitBuffer_card()// 카드 전용 버퍼
{
	glGenVertexArrays(1, &CardVAO);
	glGenBuffers(1, &CardVBO);

	glBindVertexArray(CardVBO);
	glBindBuffer(GL_ARRAY_BUFFER, CardVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Card), Card, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

}

void InitTexture_card() // 카드 전용 텍스트 버퍼
{
	int widthImage, heightImage, numberOfChannel;

	glGenTextures(1, &textures[0]);
	glBindTexture(GL_TEXTURE_2D, textures[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load("Front-1.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[1]);
	glBindTexture(GL_TEXTURE_2D, textures[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Front-2.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[2]);
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Front-3.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[3]);
	glBindTexture(GL_TEXTURE_2D, textures[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Front-4.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[4]);
	glBindTexture(GL_TEXTURE_2D, textures[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Back-1.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[5]);
	glBindTexture(GL_TEXTURE_2D, textures[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Back-2.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[6]);
	glBindTexture(GL_TEXTURE_2D, textures[6]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Back-3.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[7]);
	glBindTexture(GL_TEXTURE_2D, textures[7]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Back-4.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[8]);
	glBindTexture(GL_TEXTURE_2D, textures[8]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Left-1.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[9]);
	glBindTexture(GL_TEXTURE_2D, textures[9]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Left-2.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[10]);
	glBindTexture(GL_TEXTURE_2D, textures[10]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Left-3.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[11]);
	glBindTexture(GL_TEXTURE_2D, textures[11]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Left-4.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[12]);
	glBindTexture(GL_TEXTURE_2D, textures[12]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Right-1.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[13]);
	glBindTexture(GL_TEXTURE_2D, textures[13]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Right-2.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[14]);
	glBindTexture(GL_TEXTURE_2D, textures[14]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Right-3.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glGenTextures(1, &textures[15]);
	glBindTexture(GL_TEXTURE_2D, textures[15]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_set_flip_vertically_on_load(true);
	data = stbi_load("Right-4.png", &widthImage, &heightImage, &numberOfChannel, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, 3, widthImage, heightImage, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	stbi_image_free(data);

	glUseProgram(s_Cardprogram);
	int tLocation = glGetUniformLocation(s_Cardprogram, "outTexture");
	glUniform1i(tLocation, 1);
}


void Display()
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glViewport(0, 780, 300, 100);
	Drawtext();
	glViewport(300, 0, 1300, 900);
	glm::mat4 transformMatrix = glm::mat4(1.0f);
	glm::vec3 cameraPos = glm::vec3(cameraXYZ[0], cameraXYZ[1], cameraXYZ[2]); //--- 카메라 위치
	glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, 0.0f); //--- 카메라 바라보는 방향
	glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f); //--- 카메라 위쪽 방향
	glm::mat4 projection = glm::mat4(1.0f);
	glm::mat4 view = glm::mat4(1.0f);

	unsigned int projectionLocation;
	unsigned int modelLocation;
	unsigned int viewLocation;
	int ambientLocation;
	int lightPosLocation;
	int lightColorLocation;
	int objColorLocation;
	int viewPosition;

	viewLocation = glGetUniformLocation(s_program, "viewTransform");	//---뷰변환행렬
	projectionLocation = glGetUniformLocation(s_program, "projectionTransform");	//---투영변환행렬
	lightPosLocation = glGetUniformLocation(s_program, "lightPos");		//---조명위치
	lightColorLocation = glGetUniformLocation(s_program, "lightColor"); //---조명색깔
	viewPosition = glGetUniformLocation(s_program, "viewPos");	//--- 카메라 위치
	modelLocation = glGetUniformLocation(s_program, "modelTransform");	//---모델변환행렬
	objColorLocation = glGetUniformLocation(s_program, "objectColor"); //---오브젝트색상
	ambientLocation = glGetUniformLocation(s_program, "ambient");	//---빛 세기

	view = glm::lookAt(cameraPos, cameraDirection, cameraUp);
	projection = glm::perspective(glm::radians(65.0f), (float)g_window_w / (float)g_window_h, 0.1f, 50.0f);
	projection = glm::translate(projection, glm::vec3(0.0, 0.0, -5.0));

	glUseProgram(s_program);

	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
	glUniform3f(lightPosLocation, 0.0, 3.0, 8.0);
	glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
	glUniform3f(viewPosition, 0.0, 0.0, 4.0);
	glUniform1f(ambientLocation, 0.3);

	glBindVertexArray(ShapeVAO);
	for (int i = 0; i < mapSize; ++i) {
		for (int j = 0; j < mapSize; ++j) {
			if (j % 2 == i % 2) {
				glUniform3f(objColorLocation, 0.3, 0.3, 0.3);
			}
			else {
				glUniform3f(objColorLocation, 0.8, 0.8, 0.8);
			}

			glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Map[i][j].transform));
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
	}

	glUseProgram(s_Objprogram);
	glBindVertexArray(ObjVAO);

	viewLocation = glGetUniformLocation(s_Objprogram, "viewTransform");	//---뷰변환행렬
	projectionLocation = glGetUniformLocation(s_Objprogram, "projectionTransform");	//---투영변환행렬
	lightPosLocation = glGetUniformLocation(s_Objprogram, "lightPos");		//---조명위치
	lightColorLocation = glGetUniformLocation(s_Objprogram, "lightColor"); //---조명색깔
	viewPosition = glGetUniformLocation(s_Objprogram, "viewPos");	//--- 카메라 위치
	modelLocation = glGetUniformLocation(s_Objprogram, "modelTransform");	//---모델변환행렬
	ambientLocation = glGetUniformLocation(s_Objprogram, "ambient");	//---빛 세기

	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);
	glUniform3f(lightPosLocation, 0.0, 3.0, 8.0);
	glUniform3f(lightColorLocation, 1.0, 1.0, 1.0);
	glUniform3f(viewPosition, 0.0, 0.0, 4.0);
	glUniform1f(ambientLocation, 0.3);

	for (int i = 0; i < PLAYERNUM; ++i) {
		if (!Player1[i].checkDead())
		{
			if (Player1[i].getDirection() != STARTWARP && Player1[i].getDirection() != ENDWARP) {
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Player1[i].transform));
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, objtextures[i]);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			else {
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Player1[i].Warptransform));
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, objtextures[i]);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

		}
		else {

		}
	}

	for (int i = 0; i < PLAYERNUM; ++i) {
		if (!Player2[i].checkDead())
		{
			if (Player2[i].getDirection() != STARTWARP && Player2[i].getDirection() != ENDWARP) {
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Player2[i].transform));
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, objtextures[3 + i]);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}
			else {
				glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Player2[i].Warptransform));
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, objtextures[3 + i]);
				glDrawArrays(GL_TRIANGLES, 0, 36);
			}

		}
	}

	glUseProgram(s_Cardprogram); // 임의로 그려본 카드
	modelLocation = glGetUniformLocation(s_Cardprogram, "modelTransform");


	for (int i = 0; i < CARDNUM; ++i) {
		int cardMove = Card1.getCardMove(i);
		int cardDirection = Card1.getcardDirection(i);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Card1.transform[i]));
		glBindVertexArray(CardVAO);
		glActiveTexture(GL_TEXTURE1);
		if (cardMove != 0) {
			glBindTexture(GL_TEXTURE_2D, textures[(cardDirection - 9) * 4 + cardMove - 1]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		/*cardMove = Card2.getCardMove(i);
		cardDirection = Card2.getcardDirection(i);
		glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(Card2.transform[i]));
		glBindVertexArray(CardVAO);
		glActiveTexture(GL_TEXTURE1);

		if (cardMove != 0) {
			glBindTexture(GL_TEXTURE_2D, textures[(cardDirection - 9) * 4 + cardMove - 1]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}*/
	}


	glutSwapBuffers();
}


void Reshape(int w, int h)
{
	g_window_w = w;
	g_window_h = h;
	glViewport(0, 0, w, h);
}

void TimerFunc(int value)
{
	for (int i = 0; i < PLAYERNUM; ++i) {
		Player1[i].move(Map[Player1[i].getZ() - 1][Player1[i].getX()].getY(), Map[Player1[i].getZ() + 1][Player1[i].getX()].getY(), Map[Player1[i].getZ() - 2][Player1[i].getX()].getY(), Map[Player1[i].getZ() + 2][Player1[i].getX()].getY(), Map[Player1[i].getZ()][Player1[i].getX()].getY(), &Player2Score);
		Player2[i].move(Map[Player2[i].getZ() - 1][Player2[i].getX()].getY(), Map[Player2[i].getZ() + 1][Player2[i].getX()].getY(), Map[Player2[i].getZ() - 2][Player2[i].getX()].getY(), Map[Player2[i].getZ() + 2][Player2[i].getX()].getY(), Map[Player2[i].getZ()][Player2[i].getX()].getY(), &Player1Score);

	}

	for (int i = 0; i < mapSize; ++i)
	{
		for (int j = 0; j < mapSize; ++j)
			Map[i][j].move(0, 0, 0, 0, 0, &tmpScore);
	}

	/*if (turn % 5 == 0) {
		Card2.DistroyMap = true;
	}
	else if (turn % 5 == 1) {
		Card1.DistroyMap = true;
	}*/

	if (cameraRotateY)
		rotateCamera();

	check_collision();
	glutTimerFunc(25, TimerFunc, 1);
	glutPostRedisplay();
}


// --- [네트워크] 키보드 입력 시 서버 전송 및 ID별 제어 로직 ---
void Keyboard(unsigned char key, int x, int y)
{
	time_t timer = time(NULL);

	std::default_random_engine dreCard((unsigned int)timer);
	std::uniform_int_distribution<> uid_card{ 1, 4 };
	std::uniform_int_distribution<> uid_cardDirection{ FRONT, FALL };

	int uid_dis = uid_card(dreCard);
	int uid_dir = uid_cardDirection(dreCard);

	if (isMove) return;

	// 카드 선택 (1~5번 키)
	if (key >= '1' && key <= '5') {
		int cardIndex = key - '1'; // 0~4 인덱스 변환

		if (MoveTime) {
			if (uid_dir == FALL) uid_dir = BACK;

			// 카드 정보 가져오기
			int dis = Card1.getCardMove(cardIndex);
			int drection = Card1.getcardDirection(cardIndex);

			// --- [네트워크] g_PlayerID에 따른 분기 ---
			if (g_NetworkClient) {
				// (1) 네트워크 플레이 중일 때
				// Player 1(ID=0)인 경우
				if (g_PlayerID == 0) {
					// 내가 Player1이면 전송하고 내 말 움직임 (서버 동기화는 별도 수신 스레드에서 처리하지만, 
					// 만약 예측 이동(Local Prediction)을 원하면 여기서 changeDirection 호출)
					// 일단 서버로 보냄
					g_NetworkClient->CardSendDate(dis, drection, currentPlayer);
					// 여기서는 로컬 이동을 막고 서버 응답을 기다릴지, 아니면 바로 움직일지 결정해야 합니다.
					// 일반적으로는 서버 응답으로 움직이지만, 반응성을 위해 로컬에서 바로 움직이기도 합니다.
					// 기존 로직 유지를 위해 로컬에서도 바로 움직입니다.
					Player1[currentPlayer].changeDirection(drection, dis);
				}
				// Player 2(ID=1)인 경우
				else if (g_PlayerID == 1) {
					g_NetworkClient->CardSendDate(dis, drection, currentPlayer);
					Player2[currentPlayer].changeDirection(drection, dis);
				}
			}
			else {
				// (2) 네트워크가 아닐 때 (싱글/오프라인)
				// 기존처럼 ID 상관없이 Player1을 움직임 (혹은 테스트 로직대로)
				// 요청하신 대로 id==0이면 P1, id==1이면 P2 제어
				if (g_PlayerID == 0)
					Player1[currentPlayer].changeDirection(drection, dis);
				else if (g_PlayerID == 1)
					Player2[currentPlayer].changeDirection(drection, dis);
				// (기존 코드의 기본 동작은 Player1을 움직이는 것이었음)
			}

			// 카드 교체
			Card1.cardDelete(cardIndex);
			Card1.cardInsert(uid_dis, uid_dir);
			isMove = true;
		}

		MoveTime = false;
	}
	else {
		switch (key)
		{
		case 'A':
		case 'a': // 처음 플레이어 선택
			MoveTime = true;
			currentPlayer = 0; // 현재 플래이서 선택 넘버
			break;
		case 'S':
		case 's':
			MoveTime = true;
			currentPlayer = 1;
			break;
		case 'D':
		case 'd':
			MoveTime = true;
			currentPlayer = 2;
			break;
		case 'c':
		case 'C':
			change_card();
			break;
		case 'R':
			if (!cameraRotateMinus)
			{
				cameraRotateMinus = true;
				cameraRotateY = true;
			}
			else
				cameraRotateY = !cameraRotateY;
			break;
		case 'r':
			if (cameraRotateMinus)
			{
				cameraRotateMinus = false;
				cameraRotateY = true;
			}
			else
				cameraRotateY = !cameraRotateY;
			break;
		case 'Q':
		case 'q':
			glutLeaveMainLoop();
			break;
		default:
			break;
		}
	}
	glutPostRedisplay();
}

void InitVertices()
{
	for (int i = 0; i < 12; ++i)
	{
		shapevertices[18 * i] = cubeDots[cubeShapeindex[3 * i]][0];
		shapevertices[18 * i + 1] = cubeDots[cubeShapeindex[3 * i]][1];
		shapevertices[18 * i + 2] = cubeDots[cubeShapeindex[3 * i]][2];
		shapevertices[18 * i + 3] = cubeNormal[cubeNormalindex[3 * i]][0];
		shapevertices[18 * i + 4] = cubeNormal[cubeNormalindex[3 * i]][1];
		shapevertices[18 * i + 5] = cubeNormal[cubeNormalindex[3 * i]][2];

		shapevertices[18 * i + 6] = cubeDots[cubeShapeindex[3 * i + 1]][0];
		shapevertices[18 * i + 7] = cubeDots[cubeShapeindex[3 * i + 1]][1];
		shapevertices[18 * i + 8] = cubeDots[cubeShapeindex[3 * i + 1]][2];
		shapevertices[18 * i + 9] = cubeNormal[cubeNormalindex[3 * i + 1]][0];
		shapevertices[18 * i + 10] = cubeNormal[cubeNormalindex[3 * i + 1]][1];
		shapevertices[18 * i + 11] = cubeNormal[cubeNormalindex[3 * i + 1]][2];

		shapevertices[18 * i + 12] = cubeDots[cubeShapeindex[3 * i + 2]][0];
		shapevertices[18 * i + 13] = cubeDots[cubeShapeindex[3 * i + 2]][1];
		shapevertices[18 * i + 14] = cubeDots[cubeShapeindex[3 * i + 2]][2];
		shapevertices[18 * i + 15] = cubeNormal[cubeNormalindex[3 * i + 2]][0];
		shapevertices[18 * i + 16] = cubeNormal[cubeNormalindex[3 * i + 2]][1];
		shapevertices[18 * i + 17] = cubeNormal[cubeNormalindex[3 * i + 2]][2];
	}
}
void InitTextureVertices()
{
	for (int i = 0; i < 6; ++i)
	{
		texturevertices[12 * i] = textureST[textureindex[0]][0];
		texturevertices[12 * i + 1] = textureST[textureindex[0]][1];
		texturevertices[12 * i + 2] = textureST[textureindex[1]][0];
		texturevertices[12 * i + 3] = textureST[textureindex[1]][1];
		texturevertices[12 * i + 4] = textureST[textureindex[2]][0];
		texturevertices[12 * i + 5] = textureST[textureindex[2]][1];
		texturevertices[12 * i + 6] = textureST[textureindex[3]][0];
		texturevertices[12 * i + 7] = textureST[textureindex[3]][1];
		texturevertices[12 * i + 8] = textureST[textureindex[4]][0];
		texturevertices[12 * i + 9] = textureST[textureindex[4]][1];
		texturevertices[12 * i + 10] = textureST[textureindex[5]][0];
		texturevertices[12 * i + 11] = textureST[textureindex[5]][1];
	}
}

void change_card()
{
	time_t timer = time(NULL);
	std::default_random_engine dreCard((unsigned int)timer);
	std::uniform_int_distribution<> uid_card{ 1, 4 };
	std::uniform_int_distribution<> uid_cardDirection{ FRONT, FALL };

	int uid_dis = uid_card(dreCard);
	int uid_dir = uid_cardDirection(dreCard);

	for (int i = 0; i < CARDNUM; ++i) {
		Card1.cardDelete(i);
		Card2.cardDelete(i);

		uid_dis = uid_card(dreCard);
		uid_dir = uid_cardDirection(dreCard);

		if (uid_dir == FALL)
			uid_dir = BACK;

		Card1.cardInsert(uid_dis, uid_dir);

		uid_dis = uid_card(dreCard);
		uid_dir = uid_cardDirection(dreCard);

		if (uid_dir == FALL)
			uid_dir = FRONT;

		Card2.cardInsert(uid_dis, uid_dir);
	}
}

void Drawtext()
{
	glUseProgram(s_Textprogram);

	glRasterPos3f(-0.5, 0.0, 0.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, Player1Score + 48);
	glRasterPos3f(0.5, 0.0, 0.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, Player2Score + 48);
	glRasterPos3f(0.0, 0.0, 0.0);
	glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, ':');
}

void check_collision()
{
	// 1. Player1(i)이 Player2(j) 자리에 감 -> Player2가 죽음
	for (int i = 0; i < PLAYERNUM; ++i)
	{
		for (int j = 0; j < PLAYERNUM; ++j)
		{
			if (!Player1[i].checkDead() && !Player2[j].checkDead() && Player1[i].getX() == Player2[j].getX() && Player1[i].getZ() == Player2[j].getZ())
			{
				// --- [네트워크] ---
				// 내가 Player2(ID=1)라면, 내 말이 죽었음을 서버에 알림
				if (g_NetworkClient && g_PlayerID == 1) {
					g_NetworkClient->PlayerSendDate(1, j);
				}

				Player1Score += 1;
				Player2[j].die();
			}
		}
	}

	// 2. Player2(i)가 Player1(j) 자리에 감 -> Player1이 죽음
	for (int i = 0; i < PLAYERNUM; ++i)
	{
		for (int j = 0; j < PLAYERNUM; ++j)
		{
			if (!Player2[i].checkDead() && !Player1[j].checkDead() && Player1[i].getX() == Player2[j].getX() && Player1[i].getZ() == Player2[j].getZ())
			{
				// --- [네트워크] ---
				// 내가 Player1(ID=0)라면, 내 말이 죽었음을 서버에 알림
				if (g_NetworkClient && g_PlayerID == 0) {
					g_NetworkClient->PlayerSendDate(0, j);
				}

				Player2Score += 1;
				Player1[j].die();
			}
		}
	}

}

void InitGame()
{
	int height = 0;

	time_t timer = time(NULL);

	std::default_random_engine dre((unsigned int)timer);
	std::uniform_int_distribution<> uidHeight{ -1, 1 };
	for (int i = 0; i < mapSize; ++i) {
		for (int j = 0; j < mapSize; ++j) {
			Map[i][j].translateMatrix(-BOXSIZE * 19 + (j * 2.0 * BOXSIZE), BOXSIZE * (1 + 2 * height), -BOXSIZE * 19 + (i * 2.0 * BOXSIZE));
			Map[i][j].changeDirection(STOP, 0);
			Map[i][j].setHeight(height);
		}
		if (i > 2 && i < mapSize - 2)
		{
			if (height > 1)
				height -= 1;
			else if (height < -1)
				height += 1;
			else
				height += uidHeight(dre);
		}
	}
	for (int i = 0; i < PLAYERNUM; ++i) {
		Player2[i].initPlayer(BOXSIZE, 0);
		Player2[i].setHeight(Map[Player2[i].getZ()][Player2[i].getX()].getY());
		Player2[i].translateMatrix(0, BOXSIZE * 2 * Map[Player2[i].getZ()][Player2[i].getX()].getY(), 0);
	}

	for (int i = 0; i < PLAYERNUM; ++i) {
		Player1[i].initPlayer(BOXSIZE, 2);
		Player1[i].setHeight(Map[Player1[i].getZ()][Player1[i].getX()].getY());
		Player1[i].translateMatrix(0, BOXSIZE * 2 * Map[Player1[i].getZ()][Player1[i].getX()].getY(), 0);
	}

	for (int i = 0; i < CARDNUM; ++i) {
		Card1.translateMatrix(i, -0.9 + CARDSIZE * i * 2, -0.9, 0.0);
		Card2.translateMatrix(i, 0.9 - CARDSIZE * i * 2, 0.9, 0.0);
	}

	change_card();
}

void rotateCamera()
{
	if (cameraRotateMinus)
	{
		GLfloat tmpX = cameraXYZ[2] * glm::sin(glm::radians(-1.0f)) + cameraXYZ[0] * glm::cos(glm::radians(-1.0f));
		GLfloat tmpZ = cameraXYZ[2] * glm::cos(glm::radians(-1.0f)) - cameraXYZ[0] * glm::sin(glm::radians(-1.0f));

		cameraXYZ[0] = tmpX;
		cameraXYZ[2] = tmpZ;
	}
	else
	{
		GLfloat tmpX = cameraXYZ[2] * glm::sin(glm::radians(1.0f)) + cameraXYZ[0] * glm::cos(glm::radians(1.0f));
		GLfloat tmpZ = cameraXYZ[2] * glm::cos(glm::radians(1.0f)) - cameraXYZ[0] * glm::sin(glm::radians(1.0f));

		cameraXYZ[0] = tmpX;
		cameraXYZ[2] = tmpZ;
	}
}