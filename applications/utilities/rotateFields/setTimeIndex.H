    instantList sourceTimes = runTimeSource.times();
    label sourceTimeIndex = runTimeSource.timeIndex();
    if (args.optionFound("sourceTime"))
    {
        if (args["sourceTime"] == "latestTime")
        {
            sourceTimeIndex = sourceTimes.size() - 1;
        }
        else
        {
            sourceTimeIndex = Time::findClosestTimeIndex
            (
                sourceTimes,
                args.optionRead<scalar>("sourceTime")
            );
        }
    }
    else
    {
        sourceTimeIndex = Time::findClosestTimeIndex
        (
            sourceTimes,
            targetRunTime.time().value()
        );
    }

    runTimeSource.setTime(sourceTimes[sourceTimeIndex], sourceTimeIndex);
