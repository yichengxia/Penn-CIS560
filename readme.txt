MS1
Yilin Guo (Game Engine Tick Function and Player Physics):
In mygl, construct InputBundle to record events (keyPress, keyRelease, mouseMove) compute the delta-time and pass into player's function tick().
In player, first figure out the acceleration, base velocity and velocity limitation in each direction based on InputBundle and current player state. Following are some details:
> If player is not in FlightMode and pressed spacebar, add base vertical velocity (JumpVelocity) to m_velocity.y.
> If player's velocity is positive, its limitation is [0, MaxVelocity]; if is negative, limitation is [MinVelocity, 0]; if is zero, limitation is [MinVelocity, MaxVelocity].
> If player is not in FlightMode, it always subject to gravity (m_acceleration.y -= Gravity).
> If player is in process of transforming from GroundMode to FlightMode, add a FlyUpAcceleration to m_acceleration.y until reach FlightModeHeight.
> Player is subject to friction & drag, which is a negative velocity proportional to current velocity.
> Height of Player in flight mode is limited to [0, 255].
Then alter camera's orientation based on InputBundle, where the φ = [-89.9999, 89.9999], θ = [0, 360).
After that, move player based on current velocity and delta-time. If user is not in FlightMode, players' movement is subject to terrain collisions, which is implemented based on grid marching (rayOrigins:12 corners of the Player's collision volume model, rayDirection: forward vector of player).
If user pressed the mouse button, mygl invokes player's removeBlock()/placeBlock() function. These two functions first use grid marching to check if there is a blockHit within 3 units, if yes, then set blockHit as EMPTY/set the last empty block along the rayDirection before blockHit as STONE.