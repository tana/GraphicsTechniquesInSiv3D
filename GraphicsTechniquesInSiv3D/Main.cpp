# include <Siv3D.hpp> // OpenSiv3D v0.6.3

# include "ShadowMapping.h"

void Main()
{
	SceneManager<String> sceneManager;
	sceneManager.add<ShadowMapping>(U"ShadowMapping");

	//Window::Resize(1280, 720);
	Window::Resize(720, 720);

	while (System::Update())
	{
		if (!sceneManager.update())
		{
			break;
		}
	}
}
