#pragma once

enum class MenuAction
{
	None,
	Exit,
	OpenTexture,
	SaveTexture,
	

	Help,
	About,

	

};

// dedicated panel classes once they grow.
class MainMenuBar
{
public:

	MenuAction DrawMainMenu();


private:
	// Add any private members or methods if needed

};
