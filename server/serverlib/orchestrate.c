
static int event_loop() {

    return 0;
}

void orchestrate_frame() {

    static int last_time;

    int min_msec = 1;
    int msec;

    do
    {
        // run a few frames until we
        // hit our sync target of min_msec
        int frame_time = event_loop();
        msec = frame_time - last_time;
    } while (msec < min_msec);
    

}