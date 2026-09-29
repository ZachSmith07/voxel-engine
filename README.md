This is a voxel world generator similar to minecraft.
I'll mainly talk about the chunk generator.
The chunk generator works by blending together multiple 2D perlin noise maps.
The way this is done is that there are multiple different noises, each for a different reason.
These noises each blend to give an "average" height. The way it does this is that there are splines for each noises, which gives how much it effects the height. The sea level is 120.

Continentalness is the most important noise, as it decides where the land masses are. This gives the "base" height. This is calculated for each block by its noise value (-1 to 1), then interpolated on this graph to give the base height.
<img width="1400" height="800" alt="continentalness" src="https://github.com/user-attachments/assets/7e26a7fa-da13-44ce-86ea-ebc2f78b1bd5" />

Then, there is erosion. This is more of a complicated one, you can imagine this as a "randomness" value - how far away from the base height it can be. This is because later we change the terrain height into blocks using a 3D perlin noise generator (to allow features like cliff overhangs), the erosion gives more (or less) power to the 3D noise. With a higher erosion, it looks a lot more crazy, and you can get weird terrain generation such as floating islands.
<img width="1400" height="800" alt="erosion" src="https://github.com/user-attachments/assets/777eeabd-d164-47c1-bfff-d858f1e9df8f" />

Then there are hills/valleys (also creates rivers). The way this works, is it does the opposite of continentalness - instead of raising/lowering height.
