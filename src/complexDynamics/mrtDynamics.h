/* This file is part of the Palabos library.
 *
 * Copyright (C) 2011-2015 FlowKit Sarl
 * Route d'Oron 2
 * 1010 Lausanne, Switzerland
 * E-mail contact: contact@flowkit.com
 *
 * The most recent release of Palabos can be downloaded at 
 * <http://www.palabos.org/>
 *
 * The library Palabos is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * The library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* Orestis Malaspinas contributed this code.
 */

/** \file
 * This object is a MRT LB dynamics as described in D.Yu et al. in
 * Progress in Aerospace Sciences 39 (2003) 329-367
 *
 * The full list of implemented MRTdynamics is:
 *
 * In mrtDynamics.h:
 *
 * - MRTdynamics: "The one with rho in front of equilibrium".
 * - IncMRTdynamics: "The one with rho0 in front of equilibrium".
 *
 * In smagorinskyDynamics.h:
 *
 * - SmagorinskyMrtDynamics: "The classical, naive implementation".
 * - ConsistentSmagorinskyMRTdynamics: Consistent according to
 *       Orestis' model: doesn't mix the stress modes with the
 *       other ones when you change the relaxation time.
 * - ConsistentSmagorinskyIncMRTdynamics
 *
 * In externalForceMRTdynamics.h:
 *
 * - GuoExternalForceMRTdynamics
 * - GuoExternalForceSmagorinskyMRTdynamics 
 * - GuoExternalForceSmagorinskyIncMRTdynamics
 * - GuoExternalForceConsistentSmagorinskyMRTdynamics
 * - GuoExternalForceConsistentSmagorinskyIncMRTdynamics
 *
 */
#ifndef MRT_DYNAMICS_H
#define MRT_DYNAMICS_H

#include "core/globalDefs.h"
#include "basicDynamics/isoThermalDynamics.h"


namespace plb {

/// Implementation of the MRT collision step
template<typename T, template<typename U> class Descriptor>
class MRTdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
    /* *************** Construction / Destruction ************************ */
    MRTdynamics(T omega_);
    
    /// Clone the object on its dynamic type.
    virtual MRTdynamics<T,Descriptor>* clone() const;
    
    /// Return a unique ID for this class.
    virtual int getId() const;
    
    /* *************** Collision and Equilibrium ************************* */
    
    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                                 Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);
    
    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
private:
    static int id;
};

/// Implementation of the Anechoic MRT collision step
template<typename T, template<typename U> class Descriptor>
class AnechoicMRTdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
    /* *************** Construction / Destruction ************************ */
    AnechoicMRTdynamics(T omega_);
    
    /// Clone the object on its dynamic type.
    virtual AnechoicMRTdynamics<T,Descriptor>* clone() const;
    
    /// Return a unique ID for this class.
    virtual int getId() const;
    
    /* *************** Collision and Equilibrium ************************* */
    
    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                                 Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);
    
    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
    virtual void setDelta(T delta);
    virtual T getDelta();
    virtual void setRhoBar_target(T rhoBar_target);
    virtual T getRhoBar_target();
    virtual void setJ_target(Array<T,Descriptor<T>::d> j_target);
    virtual Array<T,Descriptor<T>::d> getJ_target();
private:
    static int id;
private:
    T delta;
    T omega;
    T rhoBar_target;
    Array<T,Descriptor<T>::d> j_target;
};

/// Implementation of incompressible MRT dynamics.
/** This is the MRT equivalent of IncBGKdynamics: the "rho" moment of the
 *  populations appears only as a pressure term in the equilibrium, while
 *  the other terms are multiplied by the constant rho0.
 **/
template<typename T, template<typename U> class Descriptor>
class IncMRTdynamics : public IsoThermalBulkDynamics<T,Descriptor> {
public:
/* *************** Construction / Destruction ************************ */
    IncMRTdynamics(T omega_);
    
    /// Clone the object on its dynamic type.
    virtual IncMRTdynamics<T,Descriptor>* clone() const;

    /// Return a unique ID for this class.
    virtual int getId() const;

/* *************** Collision and Equilibrium ************************* */

    /// Implementation of the collision step
    virtual void collide(Cell<T,Descriptor>& cell,
                         BlockStatistics& statistics_);
    
    /// Implementation of the collision step, with imposed macroscopic variables
    virtual void collideExternal(Cell<T,Descriptor>& cell, T rhoBar,
                                 Array<T,Descriptor<T>::d> const& j, T thetaBar, BlockStatistics& stat);

    /// Compute equilibrium distribution function
    virtual T computeEquilibrium(plint iPop, T rhoBar, Array<T,Descriptor<T>::d> const& j,
                                 T jSqr, T thetaBar=T()) const;
                                 
    virtual bool velIsJ() const;

/* *************** Macroscopic variables ***************************** */
    
    /// Velocity is equal to j, not u.
    virtual void computeVelocity( Cell<T,Descriptor> const& cell,
                                  Array<T,Descriptor<T>::d>& u ) const;
private:
    static int id;
};

}  // namespace plb

#endif  // MRT_DYNAMICS_H

