#pragma once

namespace Gpio
{

class Led
{
    int f_;

public:
    class State
    {
        unsigned mask_ = 0;

        friend class Led;
    public:
        bool set(unsigned port, bool on);
        bool get(unsigned port) const;
        void clean();
    };

    /*
     * @brief get Sets current GPIO state
     * @param state [IN] - GPIO ports to initialize
     * @return initialize GPIO ports specified in state
     */
    Led(const State &state);
    ~Led();

    /*
     * @brief is_open Returns status
     * @return true if driver opened successfully
     */
    bool is_open() const;

    /*
     * @brief min Returns minimal number of GPIO ports available
     * @return maximum number of GPIO ports
     */
    static unsigned min();

    /*
     * @brief max Returns maximum number of GPIO ports available
     * @return maximum number of GPIO ports
     */
    static unsigned max();

    /*
     * @brief get Sets current GPIO state
     * @param state [IN] - get current GPIO state
     * @return true if operation successful
     */
    bool get(State &state) const;

    /*
     * @brief set Sets current GPIO state
     * @param state [IN] - set GPIO ports to specified state
     * @return true if operation successful
     * 
     * Note: ports that are not initialized are ignored
     */
    bool set(const State &state) const;
};

} // namespace Gpio
