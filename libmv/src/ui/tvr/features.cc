// Copyright (c) 2007, 2008 libmv authors.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.


#include "libmv/multiview/fundamental.h"
#include "libmv/multiview/fundamental_kernel.h"
#include "libmv/multiview/robust_fundamental.h"
#include "ui/tvr/features.h"

//TODO(pau) Once stable, this has to move to libmv.
//TODO(pau) candidate should be const; we need const_iterator in Correspondence.
void ComputeFundamental(Matches &all_matches,
                        Mat3 *F,
                        Matches *consistent_matches) {
  using namespace libmv;

  // Construct matrices containing the matches.
  vector<Mat> x;
  vector<int> tracks;
  vector<int> images;
  images.push_back(0);
  images.push_back(1);
  PointMatchMatrices(all_matches, images, &tracks, &x);
  VLOG(2) << "x1\n" << x[0] << "\nx2\n" << x[1] << "\n";

  // Compute Fundamental matrix and inliers.
  vector<int> inliers;
  // TODO(pau) Expose the threshold.
  FundamentalFromCorrespondences7PointRobust(x[0], x[1], 1, F, &inliers);
  VLOG(1) << inliers.size() << " inliers\n";
  if (inliers.size() < 8) {
    return;
  }

  // Build new correspondence graph containing only inliers.
  for (int j = 0; j < inliers.size(); ++j) {
    int k = inliers[j];
    for (int i = 0; i < 2; ++i) {
      consistent_matches->Insert(images[i], tracks[k],
          all_matches.Get(images[i], tracks[k]));
    }
  }

  // Compute Fundamental matrix using all inliers.
  TwoViewPointMatchMatrices(*consistent_matches, 0, 1, &x);
  vector<Mat3> Fs;
  fundamental::kernel::NormalizedSevenPointKernel::Solve(x[0], x[1], &Fs);
  *F = Fs[0];
  NormalizeFundamental(*F, F);
  LOG(INFO) << "F:\n" << *F;
}
