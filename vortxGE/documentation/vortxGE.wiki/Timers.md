# Timers

## Getting Delta
Your game loop needs to pass the delta time to vortxGE for Animation and Physics to function correctly.
This is easy to do with lsTimer, example below.

``` c++
    // game loop
    double delta = 1.0;
    lsTimer timer;

    timer.start_Timer();
    running = true;

    while( running )
    {
	// Process system events

	// vortxGE
	vortxge->Update( delta );
	vortxge->Render();

        // Calc delta
        delta = timer.get_ElapsedTime();
        timer.start_Timer();

    }// while running
```

<- [Home](Home)

