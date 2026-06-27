#include "algorithm.h"

#include <algorithm>
#include <limits>

namespace
{
// makeSkillAction（构造技能动作）输入形式 int skillIndex 输入含义 技能下标 输出形式 Action 输出含义 USE_SKILL类型动作
Action makeSkillAction(int skillIndex)
{
    Action action;
    action.type = ActionType::USE_SKILL;
    action.skillIndex = skillIndex;
    return action;
}

// cooldownAfterUse（模拟技能冷却）输入形式 const vector<int>& currentCooldowns, const vector<Skill>& skills, int usedSkill 输入含义 当前冷却数组、技能列表、本回合使用技能下标 输出形式 vector<int> 输出含义 使用技能并经过一回合后的冷却数组
vector<int> cooldownAfterUse(const vector<int>& currentCooldowns,
                             const vector<Skill>& skills,
                             int usedSkill)
{
    vector<int> nextCooldowns = currentCooldowns;
    nextCooldowns[usedSkill] = skills[usedSkill].cooldown;
    for (int& cooldown : nextCooldowns)
    {
        if (cooldown > 0)
        {
            --cooldown;
        }
    }
    return nextCooldowns;
}

// searchBestSequence（搜索最短胜利技能序列）输入形式 int hp, int roundsLeft, const vector<Skill>& skills, const vector<int>& cooldowns, vector<int>& current, vector<int>& best 输入含义 Boss剩余血量、剩余回合、技能列表、当前冷却、当前序列、最佳序列 输出形式 bool 输出含义 true表示找到可击败Boss的序列
bool searchBestSequence(int hp,
                        int roundsLeft,
                        const vector<Skill>& skills,
                        const vector<int>& cooldowns,
                        vector<int>& current,
                        vector<int>& best)
{
    if (hp <= 0)
    {
        if (best.empty() || current.size() < best.size())
        {
            best = current;
        }
        return true;
    }
    if (roundsLeft <= 0)
    {
        return false;
    }
    if (!best.empty() && current.size() >= best.size())
    {
        return false;
    }

    bool found = false;
    vector<int> order(skills.size());
    for (int i = 0; i < static_cast<int>(skills.size()); ++i)
    {
        order[i] = i;
    }
    sort(order.begin(), order.end(), [&](int a, int b) {
        return skills[a].damage > skills[b].damage;
    });

    for (int index : order)
    {
        if (cooldowns[index] > 0)
        {
            continue;
        }

        current.push_back(index);
        vector<int> nextCooldowns = cooldownAfterUse(cooldowns, skills, index);
        if (searchBestSequence(hp - skills[index].damage,
                               roundsLeft - 1,
                               skills,
                               nextCooldowns,
                               current,
                               best))
        {
            found = true;
        }
        current.pop_back();
    }

    return found;
}

// chooseHighestDamageReadySkill（选择最高伤害可用技能）输入形式 const vector<Skill>& skills 输入含义 玩家技能列表 输出形式 int 输出含义 当前冷却为0且伤害最高的技能下标，找不到返回-1
int chooseHighestDamageReadySkill(const vector<Skill>& skills)
{
    int bestIndex = -1;
    int bestDamage = numeric_limits<int>::min();
    for (int i = 0; i < static_cast<int>(skills.size()); ++i)
    {
        if (skills[i].currentCD == 0 && skills[i].damage > bestDamage)
        {
            bestDamage = skills[i].damage;
            bestIndex = i;
        }
    }
    return bestIndex;
}
}

// Battle（Boss战算法）输入形式 const GameState& state 输入含义 当前Boss战状态 输出形式 Action 输出含义 搜索到的技能序列第一步动作，无法释放技能则为NONE
Action Algorithm::Battle(const GameState& state)
{
    if (state.boss.currentBoss < 0 ||
        state.boss.currentBoss >= static_cast<int>(state.boss.hpList.size()))
    {
        return Action{};
    }

    int hp = state.boss.hpList[state.boss.currentBoss];
    int roundsLeft = state.minRounds - state.boss.currentRound;
    if (roundsLeft <= 0)
    {
        roundsLeft = state.minRounds > 0 ? state.minRounds : 1;
    }

    vector<int> cooldowns;
    for (const Skill& skill : state.player.skills)
    {
        cooldowns.push_back(skill.currentCD);
    }

    vector<int> current;
    vector<int> best;
    searchBestSequence(hp, roundsLeft, state.player.skills, cooldowns, current, best);
    if (!best.empty())
    {
        return makeSkillAction(best.front());
    }

    int fallback = chooseHighestDamageReadySkill(state.player.skills);
    if (fallback >= 0)
    {
        return makeSkillAction(fallback);
    }

    return Action{};
}
