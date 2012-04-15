/*
 *  ExampleCircle.cpp
 *  RVO2 Library.
 *  
 *  
 *  Copyright (C) 2008-10 University of North Carolina at Chapel Hill.
 *  All rights reserved.
 *  
 *  Permission to use, copy, modify, and distribute this software and its
 *  documentation for educational, research, and non-profit purposes, without
 *  fee, and without a written agreement is hereby granted, provided that the
 *  above copyright notice, this paragraph, and the following four paragraphs
 *  appear in all copies.
 *  
 *  Permission to incorporate this software into commercial products may be
 *  obtained by contacting the University of North Carolina at Chapel Hill.
 *  
 *  This software program and documentation are copyrighted by the University of
 *  North Carolina at Chapel Hill. The software program and documentation are
 *  supplied "as is", without any accompanying services from the University of
 *  North Carolina at Chapel Hill or the authors. The University of North
 *  Carolina at Chapel Hill and the authors do not warrant that the operation of
 *  the program will be uninterrupted or error-free. The end-user understands
 *  that the program was developed for research purposes and is advised not to
 *  rely exclusively on the program for any reason.
 *  
 *  IN NO EVENT SHALL THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL OR ITS
 *  EMPLOYEES OR THE AUTHORS BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT,
 *  SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES, INCLUDING LOST PROFITS,
 *  ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF THE
 *  UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL OR THE AUTHORS HAVE BEEN ADVISED
 *  OF THE POSSIBILITY OF SUCH DAMAGE.
 *  
 *  THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL AND THE AUTHORS SPECIFICALLY
 *  DISCLAIM ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE AND ANY
 *  STATUTORY WARRANTY OF NON-INFRINGEMENT. THE SOFTWARE PROVIDED HEREUNDER IS
 *  ON AN "AS IS" BASIS, AND THE UNIVERSITY OF NORTH CAROLINA AT CHAPEL HILL AND
 *  THE AUTHORS HAVE NO OBLIGATIONS TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 *  ENHANCEMENTS, OR MODIFICATIONS.
 *  
 *  Please send all BUG REPORTS to:
 *  
 *  geom@cs.unc.edu
 *  
 *  The authors may be contacted via:
 *  
 *  Jur van den Berg, Stephen J. Guy, Jamie Snape, Ming C. Lin, and
 *  Dinesh Manocha
 *  Dept. of Computer Science
 *  Frederick P. Brooks Jr. Computer Science Bldg.
 *  3175 University of N.C.
 *  Chapel Hill, N.C. 27599-3175
 *  United States of America
 *  
 *  http://gamma.cs.unc.edu/RVO2/
 *  
 */

/*
 * Example file showing a demo with 250 agents initially positioned evenly
 * distributed on a circle attempting to move to the antipodal position on the
 * circle.
 */

#include <iostream>
#include <vector>

#if HAVE_OPENMP || _OPENMP
#include <omp.h>
#endif

#ifdef __APPLE__
#include <RVO/RVO.h>
#else
#include "RVO.h"
#endif

#ifndef M_PI
static const float M_PI = 3.14159265358979323846f;
#endif

/* Store the goals of the agents. */
std::vector<RVO::Vector2> goals;

void setupScenario(RVO::RVOSimulator* sim)
{
  /* Specify the global time step of the simulation. */
  sim->setTimeStep(0.25f);

  /* Specify the default parameters for agents that are subsequently added. */
  sim->setAgentDefaults(15.0f, 10, 10.0f, 10.0f, 1.5f, 2.0f);

  /* 
   * Add agents, specifying their start position, and store their goals on the
   * opposite side of the environment.
   */
  for (int i = 0; i < 250; ++i) {
    sim->addAgent(200.0f *
      RVO::Vector2(std::cos(i * 2.0f * M_PI / 250.0f),
      std::sin(i * 2.0f * M_PI / 250.0f)));
    goals.push_back(-sim->getAgentPosition(i));
  }
}

void updateVisualization(RVO::RVOSimulator* sim)
{
  /* Output the current global time. */
  std::cout << sim->getGlobalTime() << " ";

  /* Output the current position of all the agents. */
  for (size_t i = 0; i < sim->getNumAgents(); ++i) {
    std::cout << sim->getAgentPosition(i) << " ";
  }

  std::cout << std::endl;
}

void setPreferredVelocities(RVO::RVOSimulator* sim)
{
  /* 
   * Set the preferred velocity to be a vector of unit magnitude (speed) in the
   * direction of the goal.
   */
#pragma omp parallel for
  for (int i = 0; i < static_cast<int>(sim->getNumAgents()); ++i) {
    RVO::Vector2 goalVector = goals[i] - sim->getAgentPosition(i);
    if (RVO::absSq(goalVector) > 1.0f) {
      goalVector = RVO::normalize(goalVector);
    }
    sim->setAgentPrefVelocity(i, goalVector);
  }
}

bool reachedGoal(RVO::RVOSimulator* sim)
{
  /* Check if all agents have reached their goals. */
  for (size_t i = 0; i < sim->getNumAgents(); ++i) {
    if (RVO::absSq(sim->getAgentPosition(i) - goals[i]) > sim->getAgentRadius(i) * sim->getAgentRadius(i)) {
      return false;
    }
  }

  return true;
}

int main(int argc, const char* argv[])
{
  /* Create a new simulator instance. */
  RVO::RVOSimulator* sim = new RVO::RVOSimulator();

  /* Set up the scenario. */
  setupScenario(sim);

  /* Perform (and manipulate) the simulation. */
  do {
    updateVisualization(sim);
    setPreferredVelocities(sim);
    sim->doStep();
  } while (!reachedGoal(sim));

  delete sim;

  return 0;
}

