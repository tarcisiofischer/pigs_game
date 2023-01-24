from PyQt5.QtCore import Qt
from PyQt5.QtGui import QIcon, QPixmap, QImage, QPen, QColor
from PyQt5.QtWidgets import QApplication, QMainWindow, QToolBar, QAction, QHBoxLayout, QGraphicsScene, QVBoxLayout, \
    QGraphicsEllipseItem, QGraphicsView, QGraphicsRectItem, QGraphicsPixmapItem
import sys


class MyGraphicsView(QGraphicsView):
    def __int__(self, *args, **kwargs):
        QGraphicsView.__init__(self, *args, **kwargs)
        self.lastMousePressedPosition = None
        self.setTransformationAnchor(QGraphicsView.NoAnchor)

    def mousePressEvent(self, event):
        QGraphicsView.mousePressEvent(self, event)
        if event.button() == Qt.LeftButton:
            self.lastMousePressedPosition = event.pos()

    def mouseMoveEvent(self, event):
        QGraphicsView.mouseMoveEvent(self, event)
        if hasattr(self, "lastMousePressedPosition") and self.lastMousePressedPosition is None:
            return

        if event.buttons() & Qt.LeftButton:
            self.horizontalScrollBar().setValue(self.horizontalScrollBar().value() - (event.x() - self.lastMousePressedPosition.x()));
            self.verticalScrollBar().setValue(self.verticalScrollBar().value() - (event.y() - self.lastMousePressedPosition.y()));
            self.lastMousePressedPosition = event.pos()


class TileItem(QGraphicsPixmapItem):
    def __init__(self, pixmap, parent=None):
        super().__init__(pixmap, parent)
        self.setAcceptHoverEvents(True)
        self._is_hovered = False

    def hoverEnterEvent(self, event):
        self._is_hovered = True
        self.update()
        super().hoverEnterEvent(event)

    def hoverLeaveEvent(self, event):
        self._is_hovered = False
        self.update()
        super().hoverLeaveEvent(event)


    def paint(self, painter, option, widget=None):
        super().paint(painter, option, widget)
        if self._is_hovered:
            painter.save()
            pen = QPen(QColor("black"))
            pen.setWidth(4)
            painter.setPen(pen)
            painter.drawRect(self.boundingRect())
            painter.restore()


class MapEditorWindow(QMainWindow):

    def __init__(self):
        QMainWindow.__init__(self)
        self.setWindowTitle("Pig's Castle - Map Editor")
        self._populateToolbar()
        self.mainLayout = QVBoxLayout()

        self.scene = QGraphicsScene(-2500, -2500, 5000, 5000)
        self.view = MyGraphicsView(self.scene)

        self.setCentralWidget(self.view)
        self.setMinimumSize(400, 300)

        self.gameArea = QGraphicsRectItem(-200, -150, 400, 300)
        self.scene.addItem(self.gameArea)

    def _populateToolbar(self):
        self.toolbar = QToolBar("Toolbar")
        button_action = QAction(QIcon("assets/map_editor/new.png"), "New map", self)
        button_action.setStatusTip("Create new map")
        button_action.triggered.connect(self._onCreateNewMap)
        self.toolbar.addAction(button_action)
        self.addToolBar(self.toolbar)

    def _onCreateNewMap(self):
        print("New map")
        startPosition = self.gameArea.pos()
        for i in range(10):
            for j in range(10):
                img = QImage("assets/map_editor/new.png")
                pix = TileItem(QPixmap.fromImage(img))
                self.scene.addItem(pix)
                pix.setPos(startPosition.x() + i * img.size().width(), startPosition.y() + j * img.size().height())


if __name__ == "__main__":
    app = QApplication(sys.argv)
    main_window = MapEditorWindow()
    main_window.show()
    app.exec()
