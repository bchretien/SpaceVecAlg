// This file is part of SpaceVecAlg.
//
// SpaceVecAlg is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SpaceVecAlg is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with SpaceVecAlg.  If not, see <http://www.gnu.org/licenses/>.

#pragma once

// Don't include it directly, include SpaceVecAlg instead

namespace sva
{

// Operators implementation

template<typename Derived>
Eigen::Block<Derived, 3, Dynamic>
motionAngular(Eigen::MatrixBase<Derived>& mv)
{
	return Eigen::Block<Derived, 3, Dynamic>(mv.derived(), 0, 0, 3, mv.cols());
}

template<typename Derived>
Eigen::Block<const Derived, 3, Dynamic>
motionAngular(const Eigen::MatrixBase<Derived>& mv)
{
	return Eigen::Block<const Derived, 3, Dynamic>(mv.derived(), 0, 0, 3, mv.cols());
}

template<typename Derived>
Eigen::Block<Derived, 3, Dynamic>
motionLinear(Eigen::MatrixBase<Derived>& mv)
{
	return Eigen::Block<Derived, 3, Dynamic>(mv.derived(), 3, 0, 3, mv.cols());
}

template<typename Derived>
Eigen::Block<const Derived, 3, Dynamic>
motionLinear(const Eigen::MatrixBase<Derived>& mv)
{
	return Eigen::Block<const Derived, 3, Dynamic>(mv.derived(), 3, 0, 3, mv.cols());
}

template<typename Derived>
Eigen::Block<Derived, 3, Dynamic>
forceCouple(Eigen::MatrixBase<Derived>& mv)
{
	return Eigen::Block<Derived, 3, Dynamic>(mv.derived(), 0, 0, 3, mv.cols());
}

template<typename Derived>
Eigen::Block<const Derived, 3, Dynamic>
forceCouple(const Eigen::MatrixBase<Derived>& mv)
{
	return Eigen::Block<const Derived, 3, Dynamic>(mv.derived(), 0, 0, 3, mv.cols());
}

template<typename Derived>
Eigen::Block<Derived, 3, Dynamic>
forceForce(Eigen::MatrixBase<Derived>& mv)
{
	return Eigen::Block<Derived, 3, Dynamic>(mv.derived(), 3, 0, 3, mv.cols());
}

template<typename Derived>
Eigen::Block<const Derived, 3, Dynamic>
forceForce(const Eigen::MatrixBase<Derived>& mv)
{
	return Eigen::Block<const Derived, 3, Dynamic>(mv.derived(), 3, 0, 3, mv.cols());
}

template<typename T>
inline MotionVec<T> MotionVec<T>::cross(const MotionVec<T>& mv2) const
{
	return MotionVec<T>(angular().cross(mv2.angular()),
										 angular().cross(mv2.linear()) +
										 linear().cross(mv2.angular()));
}

template<typename T>
template<typename Derived>
inline void MotionVec<T>::cross(const Eigen::MatrixBase<Derived>& mv2,
	Eigen::MatrixBase<Derived>& result) const
{
	motionAngular(result).noalias() = motionAngular(mv2).colwise().cross(-angular_);
	motionLinear(result).noalias() = motionLinear(mv2).colwise().cross(-angular_) +
																 motionAngular(mv2).colwise().cross(-linear_);
}

template<typename T>
inline ForceVec<T> MotionVec<T>::crossDual(const ForceVec<T>& fv2) const
{
	return ForceVec<T>(angular().cross(fv2.couple()) +
										linear().cross(fv2.force()),
										angular().cross(fv2.force()));
}

template<typename T>
template<typename Derived>
inline void MotionVec<T>::crossDual(const Eigen::MatrixBase<Derived>& fv2,
	Eigen::MatrixBase<Derived>& result) const
{
	forceCouple(result).noalias() = forceCouple(fv2).colwise().cross(-angular_) +
																forceForce(fv2).colwise().cross(-linear_);
	forceForce(result).noalias() = forceForce(fv2).colwise().cross(-angular_);
}

template<typename T>
inline T MotionVec<T>::dot(const sva::ForceVec<T>& fv2) const
{
	return angular().dot(fv2.couple()) + linear().dot(fv2.force());
}

template<typename T>
inline ForceVec<T> RBInertia<T>::operator*(const MotionVec<T>& mv) const
{
	return ForceVec<T>(inertia()*mv.angular() + momentum().cross(mv.linear()),
										mass()*mv.linear() - momentum().cross(mv.angular()));
}

template<typename T>
inline ABInertia<T> ABInertia<T>::operator+(const RBInertia<T>& rbI) const
{
	using namespace Eigen;
	Matrix3<T> M, I;
	M.template triangularView<Lower>() = massMatrix() + Matrix3<T>::Identity()*rbI.mass();
	I.template triangularView<Lower>() = inertia() + rbI.inertia();
	return ABInertia<T>(M, gInertia() + vector3ToCrossMatrix(rbI.momentum()), I);
}

template<typename T>
inline ForceVec<T> ABInertia<T>::operator*(const MotionVec<T>& mv) const
{
	return ForceVec<T>(inertia()*mv.angular() + gInertia()*mv.linear(),
									massMatrix()*mv.linear() +
									gInertia().transpose()*mv.angular());
}

template<typename T>
inline MotionVec<T> PTransform<T>::operator*(const MotionVec<T>& mv) const
{
	using namespace Eigen;
	const Matrix3<T>& E = rotation();
	const Vector3<T>& r = translation();
	return MotionVec<T>(E*mv.angular(),
										 E*(mv.linear() - r.cross(mv.angular())));
}

template<typename T>
inline MotionVec<T> PTransform<T>::invMul(const MotionVec<T>& mv) const
{
	using namespace Eigen;
	const Matrix3<T>& E = rotation();
	const Vector3<T>& r = translation();
	return MotionVec<T>(E.transpose()*mv.angular(),
										 E.transpose()*mv.linear() +
										 r.cross(E.transpose()*mv.angular()));
}

template<typename T>
inline ForceVec<T> PTransform<T>::dualMul(const ForceVec<T>& fv) const
{
	using namespace Eigen;
	const Matrix3<T>& E = rotation();
	const Vector3<T>& r = translation();
	return ForceVec<T>(E*(fv.couple() - r.cross(fv.force())),
										E*fv.force());
}

template<typename T>
inline ForceVec<T> PTransform<T>::transMul(const ForceVec<T>& fv) const
{
	using namespace Eigen;
	const Matrix3<T>& E = rotation();
	const Vector3<T>& r = translation();
	Vector3<T> n(E.transpose()*fv.couple());
	n.noalias() += r.cross(E.transpose()*fv.force());
	return ForceVec<T>(n,
										E.transpose()*fv.force());
}

template<typename T>
inline RBInertia<T> PTransform<T>::dualMul(const RBInertia<T>& rbI) const
{
	using namespace Eigen;
	const Matrix3<T>& E = rotation();
	const Vector3<T>& r = translation();
	Matrix3<T> I;
	I.template triangularView<Lower>() = E*(rbI.inertia() + vector3ToCrossMatrix(r)*
			vector3ToCrossMatrix<T>(rbI.momentum()) +
			vector3ToCrossMatrix<T>(rbI.momentum() - rbI.mass()*r)*
			vector3ToCrossMatrix<T>(r))*E.transpose();
	return RBInertia<T>(rbI.mass(),
										 E*(rbI.momentum() - rbI.mass()*r),
										 I);
}

template<typename T>
inline RBInertia<T> PTransform<T>::transMul(const RBInertia<T>& rbI) const
{
	using namespace Eigen;
	const Matrix3<T>& E = rotation();
	const Vector3<T>& r = translation();
	Matrix3<T> I;
	I.template triangularView<Lower>() = E.transpose()*rbI.inertia()*E -
			vector3ToCrossMatrix<T>(r)*vector3ToCrossMatrix<T>(E.transpose()*rbI.momentum()) -
			vector3ToCrossMatrix<T>(E.transpose()*rbI.momentum() + rbI.mass()*r)*
			vector3ToCrossMatrix<T>(r);
	return RBInertia<T>(rbI.mass(), E.transpose()*rbI.momentum() + rbI.mass()*r, I);
}

template<typename T>
inline ABInertia<T> PTransform<T>::dualMul(const ABInertia<T>& rbI) const
{
	using namespace Eigen;
	const Matrix3<T>& E = rotation();
	const Vector3<T>& r = translation();

	Matrix3<T> massM =rbI.massMatrix();
	Matrix3<T> rCross = vector3ToCrossMatrix(r);
	Matrix3<T> tmpI = rbI.gInertia() - rCross*massM;

	Matrix3<T> M, I;
	M.template triangularView<Lower>() = E*massM*E.transpose();
	I.template triangularView<Lower>() = E*(rbI.inertia() - rCross*rbI.gInertia().transpose() +
			(tmpI*rCross))*E.transpose();

	return ABInertia<T>(M,
									 E*(tmpI)*E.transpose(),
									 I);
}

template<typename T>
inline ABInertia<T> PTransform<T>::transMul(const ABInertia<T>& rbI) const
{
	using namespace Eigen;
	const Matrix3<T>& E = rotation();
	const Vector3<T>& r = translation();

	Matrix3<T> Mp(E.transpose()*rbI.massMatrix()*E);
	Matrix3<T> Hp(E.transpose()*rbI.gInertia()*E);
	Matrix3<T> rCross(vector3ToCrossMatrix(r));

	Matrix3<T> M, I;
	M.template triangularView<Lower>() = Mp;
	I.template triangularView<Lower>() = (E.transpose()*rbI.inertia()*E +
																			rCross*Hp.transpose() -
																			(Hp + rCross*Mp)*rCross);
	return ABInertia<T>(M,
									 Hp + rCross*Mp,
									 I);

}

} // namespace sva
