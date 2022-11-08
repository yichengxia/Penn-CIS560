# Shiwei Ge (Procedural Terrain):
> To implement the noise function for the height of the mountain I used fractal brownian noise overtop of perlin noise. For the rolling hills I used Worley noise. To interpolate bewteen biomes I used Perlin noise with a very large grid size. When this perlin noise was above 0.5 that signified mountains and below signified rolling hills. In between 0.4 and 0.6 I interpolated with the glm::mix function to provide a smoother transition between the regions.To test the noise functions I created I modified my HW4 and using a shader. I also created two Biomes field. The first mountain field is from 0<x<32, 0<z<64. The second Grassland field is from 32<x<64, 0<z<64.If the block is above 200 and it is the top, the block will be snow with color {1,1,1}. And between 128 and 138, it will be water if it's empty.

# Yilin Guo (Game Engine Tick Function and Player Physics):

In mygl, construct InputBundle to record events (keyPress, keyRelease, mouseMove) compute the delta-time and pass into player's function tick().

In player, first figure out the acceleration, base velocity and velocity limitation in each direction based on InputBundle and current player state. Following are some details:
> If player is not in FlightMode and pressed spacebar, add base vertical velocity (JumpVelocity) to m_velocity.y.
> If player's velocity is positive, its limitation is [0, MaxVelocity]; if is negative, limitation is [MinVelocity, 0]; if is zero, limitation is [MinVelocity, MaxVelocity].
> If player is not in FlightMode, it always subject to gravity (m_acceleration.y -= Gravity).
> If player is in process of transforming from GroundMode to FlightMode, add a FlyUpAcceleration to m_acceleration.y until reach FlightModeHeight.
> Player is subject to friction & drag, which is a negative velocity proportional to current velocity.
> Height of Player in flight mode is limited to [0, 255].

Then alter camera's orientation (only when mouse is focused and the movement is not trivial) based on InputBundle, where the φ = [-89.9999, 89.9999], θ = [0, 360). 

After that, move player based on current velocity and delta-time. If user is not in FlightMode, players' movement is subject to terrain collisions, which is implemented based on grid marching (rayOrigins:12 corners of the Player's collision volume model, rayDirection: forward vector of player).

If user pressed the mouse button, mygl invokes player's removeBlock()/placeBlock() function. These two functions first use grid marching to check if there is a blockHit within 3 units, if yes, then set blockHit as EMPTY/set the last empty block along the rayDirection before blockHit as STONE.
