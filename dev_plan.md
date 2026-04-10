# Development Plan

## Setup and Initial Calibration

- After the basic shape is assembled, measure the length of each LED strip.
- Start with tentative split points at 500, 300, and 300 LEDs, then adjust from there.
- Light only the final LED in red to confirm each strip endpoint.

### Three-Way Intersection Calibration

- All LED strips start from intersection A.
- Segment 1 passes through intersections B, C, and D before looping back and finishing at C.
- Segment 2 finishes at D
- Segment 3 finished at B
- Create a calibration mode for segment 1 that:
  - Lights every 10th LED in red.
  - Marks the two LEDs where the strip should pass B and C in green.
  - Makes it easy to physically mark the exact positions.
- After all three split points are marked, define them in three arrays, one for each junction endpoint.

## V1: Constant Hamster

- Support two display modes:
  - Normal mode: a 5-LED trail moves through the maze.
  - Inverted mode: all LEDs are lit, and a dark segment moves through the maze.
- The traveling segment is referred to as the "hamster."
- In V1, there is only one traveling hamster.
- Whenever the hamster reaches a three-way intersection, it has a 1/3 chance of taking any available route:
  - One of the two new branches.
  - The route it came from.
- Speed should be globally adjustable.

## V2: Dynamic Hamster Movement

- The maze consists of tube segments that are either horizontal or vertical.
- For vertical segments, define whether the orientation is up or down, looking from the ESP toward the end of the LED segment.
- This allows the hamster to correctly interpret direction changes when moving through the maze.
- To create a more realistic portrayal of an animal moving through the maze:
  - It should accelerate at the start of a bend.
  - It should decelerate when reaching the end of a bend.
- Segment length, defined as the strip section between bends, should determine the maximum speed.
- A three-way split is also considered a bend.
- For vertical tubes:
  - Moving up should look like a struggle, with a mostly steady speed curve and small jitters.
  - Moving down should use an acceleration curve inspired by gravity, or a slightly softened version of it.
- This behavior should be implemented by storing strip segments in an array of connected pairs, along with an enumerated direction state.
- The hamster should determine its speed based on:
  - Its current position on the segment.
  - Its direction of travel.
  - The orientation of the segment.
- When reaching the end of a bend, the hamster has a 1/3 chance of reversing direction.

## V3: Multiple Hamsters

- Each hamster should move according to the V2 behavior.
- By default, two hamsters spawn at startup.
- At the start, they must move in different directions, each following one of the three strips.
- Each hamster has its own intrinsic speed, with values that are close but not identical.
- When two hamsters meet for the second time, they notice each other and one of two events happens:
  - Reproduction:
    - Both hamsters fade in sync to green over 1.5 seconds.
    - A new hamster is created.
    - After the color pulse fades back to the original color, the two parents remain paused at the meeting point for 1 more second.
    - The offspring starts moving immediately.
  - Conflict:
    - Both hamsters fade to red.
    - One hamster disappears.
    - The surviving hamster continues on its way.
- The chance of reproduction versus conflict is determined by a double-sided S-curve with a configurable threshold.
- Start with the threshold set to 7.
- Behavior of the threshold:
  - Reproduction odds are highest at low population.
  - The probability reaches 50% at a population of 7.
  - Once the population exceeds 7, the curve inverts so the odds gradually favor elimination.
  - This should naturally reduce the population over time, after which the cycle resets.

## Future Ideas

- Hamster genders
- Hamster predators
- Hamsters collecting points, Pac-Man style
- Hamsters slowly mapping their terrarium
- Hamsters with different colors and different attributes
