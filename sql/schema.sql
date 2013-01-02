SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='';

DROP SCHEMA IF EXISTS `Alder` ;
CREATE SCHEMA IF NOT EXISTS `Alder` DEFAULT CHARACTER SET latin1 ;
USE `Alder` ;

-- -----------------------------------------------------
-- Table `Alder`.`Participant`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Participant` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`Participant` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  `UId` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`Id`) ,
  UNIQUE INDEX `uqUId` (`UId` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Modality`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Modality` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`Modality` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  `Name` VARCHAR(45) NOT NULL ,
  PRIMARY KEY (`Id`) ,
  UNIQUE INDEX `uqName` (`Name` ASC) )
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Study`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Study` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`Study` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  `ParticipantId` INT UNSIGNED NOT NULL ,
  `ModalityId` INT UNSIGNED NOT NULL ,
  `Site` VARCHAR(45) NOT NULL ,
  `Interviewer` VARCHAR(45) NOT NULL ,
  `DatetimeAcquired` DATETIME NOT NULL ,
  `Note` TEXT NULL DEFAULT NULL ,
  PRIMARY KEY (`Id`) ,
  INDEX `dkDatetimeAcquired` (`DatetimeAcquired` ASC) ,
  INDEX `dkInterviewer` (`Interviewer` ASC) ,
  INDEX `fkParticipantId` (`ParticipantId` ASC) ,
  INDEX `fkModalityId` (`ModalityId` ASC) ,
  CONSTRAINT `fkStudyParticipantId`
    FOREIGN KEY (`ParticipantId` )
    REFERENCES `Alder`.`Participant` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fkStudyModalityId`
    FOREIGN KEY (`ModalityId` )
    REFERENCES `Alder`.`Modality` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Exam`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Exam` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`Exam` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  `StudyId` INT UNSIGNED NOT NULL ,
  `Laterality` ENUM('right','left','none') NOT NULL ,
  `Type` VARCHAR(255) NOT NULL ,
  PRIMARY KEY (`Id`) ,
  INDEX `fkExamStudyId` (`StudyId` ASC) ,
  INDEX `dkLaterality` (`Laterality` ASC) ,
  INDEX `dkType` (`Type` ASC) ,
  CONSTRAINT `fkExamStudyId`
    FOREIGN KEY (`StudyId` )
    REFERENCES `Alder`.`Study` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Cineloop`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Cineloop` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`Cineloop` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  `ExamId` INT UNSIGNED NOT NULL ,
  `Acquisition` INT NOT NULL ,
  PRIMARY KEY (`Id`) ,
  INDEX `fkCineloopExamId` (`ExamId` ASC) ,
  UNIQUE INDEX `uqExamIdAcquisition` (`ExamId` ASC, `Acquisition` ASC) ,
  CONSTRAINT `fkCineloopExamId`
    FOREIGN KEY (`ExamId` )
    REFERENCES `Alder`.`Exam` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Image`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Image` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`Image` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  `ExamId` INT UNSIGNED NOT NULL ,
  `Acquisition` INT NOT NULL ,
  `CineloopId` INT UNSIGNED NULL ,
  PRIMARY KEY (`Id`) ,
  INDEX `fkImageExamId` (`ExamId` ASC) ,
  INDEX `fkImageCineloopId` (`CineloopId` ASC) ,
  INDEX `dkCineloopIdframe` (`CineloopId` ASC) ,
  UNIQUE INDEX `uqExamIdAcquisition` (`ExamId` ASC, `Acquisition` ASC) ,
  CONSTRAINT `fkImageExamId`
    FOREIGN KEY (`ExamId` )
    REFERENCES `Alder`.`Exam` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fkImageCineloopId`
    FOREIGN KEY (`CineloopId` )
    REFERENCES `Alder`.`Cineloop` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`User`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`User` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`User` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  `Name` VARCHAR(255) NOT NULL ,
  `Password` VARCHAR(255) NOT NULL ,
  `LastLogin` DATETIME NULL DEFAULT NULL ,
  `StudyId` INT UNSIGNED NULL DEFAULT NULL ,
  PRIMARY KEY (`Id`) ,
  UNIQUE INDEX `uqName` (`Name` ASC) ,
  INDEX `dkLastLogin` (`LastLogin` ASC) ,
  INDEX `fkStudyId` (`StudyId` ASC) ,
  CONSTRAINT `fkUserStudyId`
    FOREIGN KEY (`StudyId` )
    REFERENCES `Alder`.`Study` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`Rating`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`Rating` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`Rating` (
  `Id` INT UNSIGNED NOT NULL AUTO_INCREMENT ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  `CineloopId` INT UNSIGNED NOT NULL ,
  `ImageId` INT UNSIGNED NOT NULL ,
  `UserId` INT UNSIGNED NOT NULL ,
  `Rating` TINYINT(1) NULL DEFAULT NULL ,
  PRIMARY KEY (`Id`) ,
  INDEX `fkImageId` (`ImageId` ASC) ,
  INDEX `fkUserId` (`UserId` ASC) ,
  INDEX `dkRating` (`Rating` ASC) ,
  INDEX `fkCineloopId` (`CineloopId` ASC) ,
  UNIQUE INDEX `uqImageIdUserId` (`ImageId` ASC, `UserId` ASC) ,
  UNIQUE INDEX `uqCineloopIdUserId` (`CineloopId` ASC, `UserId` ASC) ,
  CONSTRAINT `fkRatingImageId`
    FOREIGN KEY (`ImageId` )
    REFERENCES `Alder`.`Image` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fkRatingUserId`
    FOREIGN KEY (`UserId` )
    REFERENCES `Alder`.`User` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fkRatingCineloopId`
    FOREIGN KEY (`CineloopId` )
    REFERENCES `Alder`.`Cineloop` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `Alder`.`UserHasModality`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `Alder`.`UserHasModality` ;

CREATE  TABLE IF NOT EXISTS `Alder`.`UserHasModality` (
  `UserId` INT UNSIGNED NOT NULL ,
  `ModalityId` INT UNSIGNED NOT NULL ,
  `UpdateTimestamp` TIMESTAMP NOT NULL ,
  `CreateTimestamp` TIMESTAMP NOT NULL ,
  PRIMARY KEY (`UserId`, `ModalityId`) ,
  INDEX `fkModalityId` (`ModalityId` ASC) ,
  INDEX `fkUserId` (`UserId` ASC) ,
  CONSTRAINT `fkUserHasModalityUserId`
    FOREIGN KEY (`UserId` )
    REFERENCES `Alder`.`User` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fkUserHasModalityModalityId`
    FOREIGN KEY (`ModalityId` )
    REFERENCES `Alder`.`Modality` (`Id` )
    ON DELETE NO ACTION
    ON UPDATE NO ACTION)
ENGINE = InnoDB;



SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
