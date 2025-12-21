#include "Stage.hpp"
#include <algorithm>
#include <numbers>
#include <iostream>

using Xeryon::Stage;

std::string Stage::removeWhitespace(const std::string &str)
{
    std::string result;
    result.reserve(str.size());
    std::copy_if(str.begin(), str.end(), std::back_inserter(result),
                 [](char c)
                 { return !std::isspace(c); });
    return result;
}

Stage Stage::create(Type type)
{
    constexpr double PI = M_PI;

    switch (type)
    {
    case Type::XLS_312:
        return Stage(type, true, "XLS1=312", 312.5, 1000);
    case Type::XLS_1250:
        return Stage(type, true, "XLS1=1251", 1250, 1000);
    case Type::XLS_1250_OLD:
        return Stage(type, true, "XLS1=1250", 1250, 1000);
    case Type::XLS_1250_OLD_2:
        return Stage(type, true, "XLS1=1250", 312.5, 1000);
    case Type::XLS_78:
        return Stage(type, true, "XLS1=78", 78.125, 1000);
    case Type::XLS_5:
        return Stage(type, true, "XLS1=5", 5, 1000);
    case Type::XLS_1:
        return Stage(type, true, "XLS1=1", 1, 1000);
    case Type::XLS_312_3N:
        return Stage(type, true, "XLS3=312", 312.5, 1000);
    case Type::XLS_1250_3N:
        return Stage(type, true, "XLS3=1251", 1250, 1000);
    case Type::XLS_1250_3N_OLD:
        return Stage(type, true, "XLS3=1250", 312.5, 1000);
    case Type::XLS_78_3N:
        return Stage(type, true, "XLS3=78", 78.125, 1000);
    case Type::XLS_5_3N:
        return Stage(type, true, "XLS3=5", 5, 1000);
    case Type::XLS_1_3N:
        return Stage(type, true, "XLS3=1", 1, 1000);
    case Type::XLA_312:
        return Stage(type, true, "XLA1=312", 312.5, 1000);
    case Type::XLA_1250:
        return Stage(type, true, "XLA1=1250", 1250, 1000);
    case Type::XLA_78:
        return Stage(type, true, "XLA1=78", 78.125, 1000);
    case Type::XLA_OL:
        return Stage(type, true, "XLA1=0", 1, 1000);
    case Type::XLA_OL_3N:
        return Stage(type, true, "XLA3=0", 1, 1000);
    case Type::XLA_312_3N:
        return Stage(type, true, "XLA3=312", 312.5, 1000);
    case Type::XLA_1250_3N:
        return Stage(type, true, "XLA3=1250", 1250, 1000);
    case Type::XLA_78_3N:
        return Stage(type, true, "XLA3=78", 78.125, 1000);
    case Type::XLA_312_5N:
        return Stage(type, true, "XLA3=312", 312.5, 1000);
    case Type::XLA_1250_5N:
        return Stage(type, true, "XLA3=1250", 1250, 1000);
    case Type::XLA_78_5N:
        return Stage(type, true, "XLA3=78", 78.125, 1000);
    case Type::XLA_312_10N:
        return Stage(type, true, "XLA3=312", 312.5, 1000);
    case Type::XLA_1250_10N:
        return Stage(type, true, "XLA3=1250", 1250, 1000);
    case Type::XLA_78_10N:
        return Stage(type, true, "XLA3=78", 78.125, 1000);
    case Type::XLA_312_OLD:
        return Stage(type, true, "XLA=312", 312.5, 1000);
    case Type::XLA_1250_OLD:
        return Stage(type, true, "XLA=1250", 1250, 1000);
    case Type::XLA_78_OLD:
        return Stage(type, true, "XLA=78", 78.125, 1000);
    case Type::XRTA:
        return Stage(type, false, "XRTA=109", (2 * PI * 1e6) / 57600, 100);
    case Type::XRTU_40_3:
        return Stage(type, false, "XRT1=2", (2 * PI * 1e6) / 2764800, 100);
    case Type::XRTU_40_19:
        return Stage(type, false, "XRT1=18", (2 * PI * 1e6) / 345600, 100);
    case Type::XRTU_40_49:
        return Stage(type, false, "XRT1=47", (2 * PI * 1e6) / 135000, 100);
    case Type::XRTU_40_109:
        return Stage(type, false, "XRT1=73", (2 * PI * 1e6) / 86400, 100);
    case Type::XRTU_30_3:
        return Stage(type, false, "XRT1=3", (2 * PI * 1e6) / 1843200, 100);
    case Type::XRTU_30_19:
        return Stage(type, false, "XRT1=19", (2 * PI * 1e6) / 360000, 100);
    case Type::XRTU_30_49:
        return Stage(type, false, "XRT1=49", (2 * PI * 1e6) / 144000, 100);
    case Type::XRTU_30_109:
        return Stage(type, false, "XRT1=109", (2 * PI * 1e6) / 57600, 100);
    case Type::XRTU_60_3:
        return Stage(type, false, "XRT3=3", (2 * PI * 1e6) / 2073600, 100);
    case Type::XRTU_60_19:
        return Stage(type, false, "XRT3=19", (2 * PI * 1e6) / 324000, 100);
    case Type::XRTU_60_49:
        return Stage(type, false, "XRT3=49", (2 * PI * 1e6) / 129600, 100);
    case Type::XRTU_60_109:
        return Stage(type, false, "XRT3=109", (2 * PI * 1e6) / 64800, 100);
    case Type::XRTU_30_109_OLD:
        return Stage(type, false, "XRTU=109", (2 * PI * 1e6) / 57600, 100);
    case Type::XRTU_40_73_OLD:
        return Stage(type, false, "XRTU=73", (2 * PI * 1e6) / 86400, 100);
    case Type::XRTU_40_3_OLD:
        return Stage(type, false, "XRTU=3", (2 * PI * 1e6) / 1800000, 100);
    }

    // Should never reach here
    throw std::runtime_error("Invalid stage type");
}

std::vector<Stage> Stage::getAllStages()
{
    std::vector<Stage> stages;
    stages.reserve(47); // Total number of stage types

    // Add all stage types
    stages.push_back(create(Type::XLS_312));
    stages.push_back(create(Type::XLS_1250));
    stages.push_back(create(Type::XLS_1250_OLD));
    stages.push_back(create(Type::XLS_1250_OLD_2));
    stages.push_back(create(Type::XLS_78));
    stages.push_back(create(Type::XLS_5));
    stages.push_back(create(Type::XLS_1));
    stages.push_back(create(Type::XLS_312_3N));
    stages.push_back(create(Type::XLS_1250_3N));
    stages.push_back(create(Type::XLS_1250_3N_OLD));
    stages.push_back(create(Type::XLS_78_3N));
    stages.push_back(create(Type::XLS_5_3N));
    stages.push_back(create(Type::XLS_1_3N));
    stages.push_back(create(Type::XLA_312));
    stages.push_back(create(Type::XLA_1250));
    stages.push_back(create(Type::XLA_78));
    stages.push_back(create(Type::XLA_OL));
    stages.push_back(create(Type::XLA_OL_3N));
    stages.push_back(create(Type::XLA_312_3N));
    stages.push_back(create(Type::XLA_1250_3N));
    stages.push_back(create(Type::XLA_78_3N));
    stages.push_back(create(Type::XLA_312_5N));
    stages.push_back(create(Type::XLA_1250_5N));
    stages.push_back(create(Type::XLA_78_5N));
    stages.push_back(create(Type::XLA_312_10N));
    stages.push_back(create(Type::XLA_1250_10N));
    stages.push_back(create(Type::XLA_78_10N));
    stages.push_back(create(Type::XLA_312_OLD));
    stages.push_back(create(Type::XLA_1250_OLD));
    stages.push_back(create(Type::XLA_78_OLD));
    stages.push_back(create(Type::XRTA));
    stages.push_back(create(Type::XRTU_40_3));
    stages.push_back(create(Type::XRTU_40_19));
    stages.push_back(create(Type::XRTU_40_49));
    stages.push_back(create(Type::XRTU_40_109));
    stages.push_back(create(Type::XRTU_30_3));
    stages.push_back(create(Type::XRTU_30_19));
    stages.push_back(create(Type::XRTU_30_49));
    stages.push_back(create(Type::XRTU_30_109));
    stages.push_back(create(Type::XRTU_60_3));
    stages.push_back(create(Type::XRTU_60_19));
    stages.push_back(create(Type::XRTU_60_49));
    stages.push_back(create(Type::XRTU_60_109));
    stages.push_back(create(Type::XRTU_30_109_OLD));
    stages.push_back(create(Type::XRTU_40_73_OLD));
    stages.push_back(create(Type::XRTU_40_3_OLD));

    return stages;
}

std::optional<Stage> Stage::getStage(const std::string &stageCommand)
{
    std::string cleanCommand = removeWhitespace(stageCommand);

    for (const auto &stage : getAllStages())
    {
        std::string cleanStageCommand = removeWhitespace(stage.getEncoderResolutionCommand());
        if (cleanCommand.find(cleanStageCommand) != std::string::npos ||
            cleanStageCommand.find(cleanCommand) != std::string::npos)
        {
            return stage;
        }
    }

    return std::nullopt;
}