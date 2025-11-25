//TeamProject.cpp
#define  _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_IMPLEMENTATION

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <random>
#include "function.h"
#include "objRead.h"
#include "makeShader.h"
// [추가] 김윤기: 네트워크 클라이언트 헤더 포함
#include "GameClient.h"

using namespace std;

// [추가] 김윤기: 전역 클라이언트 객체 정의 (function.cpp의 extern과 연결됨)
GameClient* g_NetworkClient = nullptr;

int main(int argc, char** argv)
{
	// create window using freeglut
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(g_window_w, g_window_h);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Black & White");

	//////////////////////////////////////////////////////////////////////////////////////
	//// initialize GLEW
	//////////////////////////////////////////////////////////////////////////////////////
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK)
	{
		std::cerr << "Unable to initialize GLEW ... exiting" << std::endl;
		exit(EXIT_FAILURE);
	}
	else
	{
		std::cout << "GLEW OK\n";
	}
	//////////////////////////////////////////////////////////////////////////////////////
	//// Create shader program an register the shader
	//////////////////////////////////////////////////////////////////////////////////////
	vShader = MakeVertexShader("vertex_object.glsl", 0);
	fShader = MakeFragmentShader("fragment_object.glsl", 0);

	vCardShader = MakeVertexShader("vertex_card.glsl", 0);
	fCardShader = MakeFragmentShader("fragment_card.glsl", 0);

	vObjShader = MakeVertexShader("vertex_texture_object.glsl", 0);
	fObjShader = MakeFragmentShader("fragment_texture_object.glsl", 0);

	vTextShader = MakeVertexShader("vertex_text.glsl", 0);
	fTextShader = MakeFragmentShader("fragment_text.glsl", 0);

	// shader Program
	s_program = glCreateProgram();
	glAttachShader(s_program, vShader);
	glAttachShader(s_program, fShader);
	glLinkProgram(s_program);
	checkCompileErrors(s_program, "PROGRAM");

	s_Cardprogram = glCreateProgram();
	glAttachShader(s_Cardprogram, vCardShader);
	glAttachShader(s_Cardprogram, fCardShader);
	glLinkProgram(s_Cardprogram);
	checkCompileErrors(s_Cardprogram, "PROGRAM_card");

	s_Objprogram = glCreateProgram();
	glAttachShader(s_Objprogram, vObjShader);
	glAttachShader(s_Objprogram, fObjShader);
	glLinkProgram(s_Objprogram);
	checkCompileErrors(s_Objprogram, "PROGRAM_obj");

	s_Textprogram = glCreateProgram();
	glAttachShader(s_Textprogram, vTextShader);
	glAttachShader(s_Textprogram, fTextShader);
	glLinkProgram(s_Textprogram);

	checkCompileErrors(s_Objprogram, "PROGRAM_obj");
	InitVertices();
	InitTextureVertices();
	InitBuffer();
	InitBuffer_card();
	InitTexture_card();
	InitBuffer_obj();
	InitTexture_obj();
	InitGame();
	
	// callback functions
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Keyboard);
	glutTimerFunc(25, TimerFunc, 1);

	// ---------------------------------------------------------------
	// [김윤기 담당 파트] 네트워크 연결 시작
	// ---------------------------------------------------------------
	// IP주소: "127.0.0.1" (로컬 테스트용), 포트: 9000 (서버와 동일하게)
	// 실제 다른 컴퓨터와 하려면 서버 컴퓨터의 IP를 적어야 함.
	g_NetworkClient = new GameClient("127.0.0.1", 9000);

	if (g_NetworkClient->ConnectToServer()) {
		std::cout << ">>> [메인] 서버 접속 성공! 게임 동기화 시작. <<<" << std::endl;
	}
	else {
		std::cout << ">>> [메인] 서버 접속 실패! 싱글 모드로 동작합니다. <<<" << std::endl;
	}
	// ---------------------------------------------------------------

	// freeglut 윈도우 이벤트 처리 시작. 윈도우가 닫힐때까지 무한루프 실행.
	glutMainLoop();

	// 종료 시 메모리 해제
	if (g_NetworkClient) delete g_NetworkClient;

	return 0;
}