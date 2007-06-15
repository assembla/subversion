from csvn.core import *
from ctypes import *
import csvn.types as _types
import os

class WC(object):
    """A SVN working copy."""

    def __init__(self, path="", user=None):
        """Open a working copy directory relative to PATH"""

        self.pool = Pool()
        self.iterpool = Pool()
        self.path = path

        self.client = POINTER(svn_client_ctx_t)()
        svn_client_create_context(byref(self.client), self.pool)
        self._as_parameter_ = POINTER(svn_ra_session_t)()

    def copy(self, src, dest, rev = ""):
        """Copy SRC to DEST"""

        opt_rev = svn_opt_revision_t()
        dummy_rev = svn_opt_revision_t()
        svn_opt_parse_revision(byref(opt_rev), byref(dummy_rev),
                               str(rev), self.iterpool)

        svn_client_copy3(NULL,
                         self._build_path(src),
                         byref(opt_rev),
                         self._build_path(dest),
                         self.client, self.iterpool)

        self.iterpool.clear()

    def move(self, src, dest, force = False):
        """Move SRC to DEST"""

        svn_client_move3(NULL,
                         self._build_path(src),
                         self._build_path(dest),
                         force,
                         self.client, self.iterpool)
        self.iterpool.clear()

    def delete(self, paths, force = False):
        """Schedule PATHS to be deleted"""

        svn_client_delete2(NULL, self._build_path_list(paths),
                           force, self.client, self.iterpool)
        self.iterpool.clear()

    def add(self, path, recurse = True, force = False, no_ignore = False):
        """Schedule PATH to be added"""

        svn_client_add3(self._build_path(path),
                        recurse, force, no_ignore, self.client,
                        self.iterpool)
        self.iterpool.clear()

    def resolve(self, path, recurse = True):
        """Resolve a conflict on PATH"""

        svn_client_resolved(path, recurse, self.client, self.iterpool)
        self.iterpool.clear()

    def revert(self, paths, recurse = False):
        """Revert PATHS to the most recent version. If RECURSE is TRUE, the
        revert will recurse through directories."""

        svn_client_revert(self._build_path_list(paths), recurse,
                          self.client, self.iterpool)
        self.iterpool.clear()

    def _build_path_list(self, paths):
        """Build a list of canonicalized WC paths"""
        if isinstance(paths, (str, unicode, String)):
            paths = [paths]
        canonicalized_paths = [self._build_path(path) for path in paths]
        return _types.Array(String, canonicalized_paths)

    def _build_path(self, path):
        """Build a canonicalized path to an item in the WC"""
        return svn_path_canonicalize(os.path.join(self.path, path),
                                     self.iterpool)

