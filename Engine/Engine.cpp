#include "Engine.h"

HDC       g_HDC;					// globalny kontekst urz�dzenia
int Engine::g_height = 600,
	Engine::g_width = 800;
bool Engine::g_fullScreen = false;
float Engine::g_aspect;

// Objects from Engine
CTextFont defaultTextFont;
CTimer GTimer;

bool InitializeOpenGL()
{
	//glEnable( GL_SMOOTH );		// Cieniowanie g�adkie
	//glEnable( GL_LIGHTING );	// O�wietlenie
	//glEnable( GL_NORMALIZE );	// Normalizacja (u�ywa wektor�w normalnych przy o�wietleniu)
	glEnable( GL_CULL_FACE );	// Ukrywa tylne �cianki obiektu
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_TEXTURE_2D );	// Uruchomienie teksturowania
	glClearColor( 0, 0, 0, 1 );

	defaultTextFont.create( L"courier", 20 );

	return true;
}

// funkcja okre�laj�ca format pikseli
void SetupPixelFormat(HDC hDC)
{
	int nPixelFormat;					// indeks formatu pikseli

	static PIXELFORMATDESCRIPTOR pfd = {
		sizeof(PIXELFORMATDESCRIPTOR),	// rozmiar struktury
		1,								// domy�lna wersja
		PFD_DRAW_TO_WINDOW |			// grafika w oknie
		PFD_SUPPORT_OPENGL |			// grafika OpenGL 
		PFD_DOUBLEBUFFER,				// podw�jne buforowanie
		PFD_TYPE_RGBA,					// tryb kolor�w RGBA 
		32,								// 32-bitowy opis kolor�w
		0, 0, 0, 0, 0, 0,				// nie specyfikuje bit�w kolor�w
		0,								// bez buforu alfa
		0,								// nie specyfikuje bitu przesuni�cia
		0,								// bez bufora akumulacji
		0, 0, 0, 0,						// ignoruje bity akumulacji
		16,								// 16-bit bufor z
		0,								// bez bufora powielania
		0,								// bez bufor�w pomocniczych
		PFD_MAIN_PLANE,					// g��wna p�aszczyzna rysowania
		0,								// zarezerwowane
		0, 0, 0 };						// ingoruje maski warstw

	nPixelFormat = ChoosePixelFormat(hDC, &pfd);	// wybiera najbardziej zgodny format pikseli 

	SetPixelFormat(hDC, nPixelFormat, &pfd);		// okre�la format pikseli dla danego kontekstu urz�dzenia
}

// punkt, w kt�rym rozpoczyna si� wykonywanie aplikacji
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	Engine::preInitialize();

	WNDCLASSEX windowClass;		// klasa okna
	HWND	   hwnd;			// uchwyt okna
	MSG		   msg;				// komunikat
	bool	   done;			// znacznik zako�czenia aplikacji
	DWORD	   dwExStyle;						// rozszerzony styl okna
	DWORD	   dwStyle;						// styl okna
	RECT	   windowRect;

	// zmienne pomocnicze
	int bits = 32;

	windowRect.left=(long)0;						// struktura okre�laj�ca rozmiary okna
	windowRect.right=(long)Engine::g_width;					
	windowRect.top=(long)0;						
	windowRect.bottom=(long)Engine::g_height;					

	// definicja klasy okna
	windowClass.cbSize			= sizeof(WNDCLASSEX);
	windowClass.style			= CS_HREDRAW | CS_VREDRAW;
	windowClass.lpfnWndProc		= WndProc;
	windowClass.cbClsExtra		= 0;
	windowClass.cbWndExtra		= 0;
	windowClass.hInstance		= hInstance;
	windowClass.hIcon			= LoadIcon(NULL, IDI_APPLICATION);	// domy�lna ikona
	windowClass.hCursor			= LoadCursor(NULL, IDC_ARROW);		// domy�lny kursor
	windowClass.hbrBackground	= NULL;								// bez t�a
	windowClass.lpszMenuName	= NULL;								// bez menu
	windowClass.lpszClassName	= L"MojaKlasa";
	windowClass.hIconSm			= LoadIcon(NULL, IDI_WINLOGO);		// logo Windows

	// rejestruje klas� okna
	if (!RegisterClassEx(&windowClass))
		return 0;

	if (Engine::g_fullScreen)								// tryb pe�noekranowy?
	{
		DEVMODE dmScreenSettings;					// tryb urz�dzenia
		memset(&dmScreenSettings,0,sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);	
		dmScreenSettings.dmPelsWidth = Engine::g_width;			// szeroko�� ekranu
		dmScreenSettings.dmPelsHeight = Engine::g_height;			// wysoko�� ekranu
		dmScreenSettings.dmBitsPerPel = bits;				// bit�w na piksel
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

		// 
		if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			// prze��czenie trybu nie powiod�o si�, z powrotem tryb okienkowy
			MessageBox(NULL, L"Prze��czenie trybu wyswietlania nie powiod�o si�", NULL, MB_OK);
			Engine::g_fullScreen=FALSE;	
		}
	}

	if (Engine::g_fullScreen)								// tryb pe�noekranowy?
	{
		dwExStyle=WS_EX_APPWINDOW;					// rozszerzony styl okna
		dwStyle=WS_POPUP;						// styl okna
		ShowCursor(FALSE);						// ukrywa kursor myszy
	}
	else
	{
		dwExStyle=WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;			// definicja klasy okna
		dwStyle=WS_OVERLAPPEDWINDOW;					// styl okna
	}

	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);		// koryguje rozmiar okna

	// tworzy okno
	hwnd = CreateWindowEx(NULL,									// styl rozszerzony
						  L"MojaKlasa",			// nazwa klasy
						  L"Engine",		// app name
						  dwStyle | WS_CLIPCHILDREN |
						  WS_CLIPSIBLINGS,
						  0, 0,								// wsp�rz�dne x,y
						  windowRect.right - windowRect.left,
						  windowRect.bottom - windowRect.top, // szeroko��, wysoko��
						  NULL,									// uchwyt okna nadrz�dnego
						  NULL,									// uchwyt menu
						  hInstance,							// instancja aplikacji
						  NULL);								// no extra params

	// sprawdza, czy utworzenie okna nie powiod�o si� (wtedy warto�� hwnd r�wna NULL)
	if (!hwnd)
		return 0;

	ShowWindow(hwnd, SW_SHOW);			// wy�wietla okno
	UpdateWindow(hwnd);					// aktualizuje okno

	done = false;						// inicjuje zmienn� warunku p�tli
	InitializeOpenGL();			// inicjuje OpenGL
	Engine::initialize();

	// p�tla przetwarzania komunikat�w
	while (!done)
	{
		PeekMessage(&msg, hwnd, NULL, NULL, PM_REMOVE);

		if (msg.message == WM_QUIT)		// aplikacja otrzyma�a komunikat WM_QUIT?
		{
			done = true;				// je�li tak, to ko�czy dzia�anie
		}
		else
		{
			CInput::Update();
			GTimer.update();

			if( GStateManager::update() == false )
				done = true;

			if( GStateManager::isActive == false )
			{
				if( Engine::update() == false )
					done = true;
			}

			glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
			glLoadIdentity();
			glTranslatef( -Engine::g_aspect, 1.0f, -40.0f );			// Ustawia uk�ad wsp�rz�dnych na lewy g�rny r�g

			if( GStateManager::isActive )
			{
				GStateManager::render();
			}
			else
			{
				Engine::render();
			}

			SwapBuffers(g_HDC);			// prze��cza bufory

			TranslateMessage(&msg);		// t�umaczy komunikat i wysy�a do systemu
			DispatchMessage(&msg);
		}
	}

	if (Engine::g_fullScreen)
	{
		ChangeDisplaySettings(NULL,0);					// If So Switch Back To The Desktop
		ShowCursor(TRUE);						// Show Mouse Pointer
	}

	Engine::shutdown();

	return msg.wParam;
}

// procedura okienkowa
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HGLRC hRC;					// kontekst tworzenia grafiki
	static HDC hDC;						// kontekst urz�dzenia
	int width, height;					// szeroko�� i wysoko�� okna

	switch(message)
	{
		case WM_CREATE:					// okno jest tworzone

			hDC = GetDC(hwnd);			// pobiera kontekst urz�dzenia dla okna
			g_HDC = hDC;
			SetupPixelFormat(hDC);		// wywo�uje funkcj� okre�laj�c� format pikseli

         // tworzy kontekst tworzenia grafiki i czyni go bie��cym
			hRC = wglCreateContext(hDC);
			wglMakeCurrent(hDC, hRC);

			return 0;
			break;

		case WM_CLOSE:					// okno jest zamykane

			// deaktywuje bie��cy kontekst tworzenia grafiki i usuwa go
			wglMakeCurrent(hDC, NULL);
			wglDeleteContext(hRC);

			// wstawia komunikat WM_QUIT do kolejki
			PostQuitMessage(0);

			return 0;
			break;

		case WM_SIZE:
			height = HIWORD(lParam);		// pobiera nowe rozmiary okna
			width = LOWORD(lParam);

			if (height==0)					// unika dzielenie przez 0
			{
				height=1;					
			}

			Engine::g_width = width;
			Engine::g_height = height;

			glViewport(0, 0, width, height);		// nadaje nowe wymairy oknu OpenGL
			glMatrixMode(GL_PROJECTION);			// wybiera macierz rzutowania
			glLoadIdentity();						// resetuje macierz rzutowania

			// wyznacza proporcje obrazu
			Engine::g_aspect = (GLfloat)width/(GLfloat)height;

			//gluPerspective(54.0f,g_aspect,1.0f,1000.0f);
			glOrtho(-Engine::g_aspect, Engine::g_aspect, -1.0, 1.0, 5.0, 100.0);

			glMatrixMode(GL_MODELVIEW);				// wybiera macierz modelowania
			glLoadIdentity();						// resetuje macierz modelowania

			return 0;
			break;

		case WM_KEYDOWN:
			{
				CInput::KeyDown( tolower( (unsigned char)wParam ) );
			}
			break;

		case WM_KEYUP:
			{
				CInput::KeyUp( tolower( (unsigned char)wParam ) );
			}
			break;

		case WM_MOUSEMOVE:
			{
				CInput::MouseMotion( LOWORD(lParam), HIWORD(lParam) );
			}
			break;

		default:
			break;
	}

	return (DefWindowProc(hwnd, message, wParam, lParam));
}

