#include <Animation.hpp>
#include <utility>

Animation::Animation(
	SDL_Texture *spritesheet,
	std::vector<std::tuple<int, int> > frames,
	int framesize_x,
	int framesize_y,
	double animation_time
)
	: spritesheet(spritesheet)
	, frames(std::move(frames))
	, state(0)
	, counter(0)
	, framesize_x(framesize_x)
	, framesize_y(framesize_y)
	, animation_time(animation_time)
{
}
