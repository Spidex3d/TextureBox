#include <miniBoxLog.h>
#include <App.h>


int main() {
	// this is all the code that runs when the application starts
	App app;

	if (!app.Init()) {
		BOX_LOG_DEBUG("Failed to initialize BoxEditor");
		return -1;
	}

	BOX_LOG_INFO("Starting TextureBox application...");

	return app.Run();
}