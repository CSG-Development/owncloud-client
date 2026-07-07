#pragma once

namespace APP {

class OnboardingState
{
public:
    static constexpr int PageCount = 5;

    bool shouldShow() const;
    int nextPage() const;

    void markPageShown(int page);
    void markCompleted();
    void markDismissed();

private:
    bool installerRequiresOnboarding() const;
};

} // namespace APP
